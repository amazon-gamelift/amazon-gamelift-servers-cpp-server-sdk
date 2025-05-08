/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */
#include <aws/gamelift/internal/model/Message.h>
#include <aws/gamelift/internal/model/ResponseMessage.h>
#include <aws/gamelift/internal/network/WebSocketppClientWrapper.h>
#include <aws/gamelift/internal/retry/GeometricBackoffRetryStrategy.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>
#include <memory>
#include <websocketpp/error.hpp>

namespace Aws {
namespace GameLift {
namespace Internal {

WebSocketppClientWrapper::WebSocketppClientWrapper(std::shared_ptr<WebSocketppClientType> webSocketClient)
    : m_webSocketClient(webSocketClient), m_connectionStateChanged(false) {
    // configure logging. comment these out to get websocket logs on stdout for debugging
    m_webSocketClient->clear_access_channels(websocketpp::log::alevel::all);
    m_webSocketClient->clear_error_channels(websocketpp::log::elevel::all);

    // initialize ASIO
    m_webSocketClient->init_asio();

    // start in perpetual mode (do not exit processing loop when there are no connections)
    m_webSocketClient->start_perpetual();

    // Kick off two threads that will own up to two parallel connections. These threads will live
    // until the SDK is shutdown and alternate handling connections. If a connection fails to open,
    // the thread will not hang. Instead, the thread will simply wait for another connection to
    // appear in queue.
    //
    // Flow of logic in the two threads:
    // --- SDK initialized ---
    // socket_thread_1: waiting for connection...
    // socket_thread_2: waiting for connection...
    // --- Initial connection happens ---
    // socket_thread_1: handling 1st connection
    // socket_thread_2: waiting for connection...
    // --- Connection refresh begins ---
    // socket_thread_1: finish handling 1st connection messages
    // socket_thread_2: handling 2nd connection
    // --- Connection 1 closes ---
    // socket_thread_1: waiting for connection...
    // socket_thread_2: handling 2nd connection
    // --- SDK shut down, and WebSocket client "->stop_perpetual()" is invoked ---
    // socket_thread_1: No longer waits for a connection, thread ends
    // socket_thread_2: Finishes handling 2nd connection, then thread ends
    m_socket_thread_1 = std::unique_ptr<std::thread>(new std::thread([this] { m_webSocketClient->run(); }));
    m_socket_thread_2 = std::unique_ptr<std::thread>(new std::thread([this] { m_webSocketClient->run(); }));

    // Set callbacks
    using std::placeholders::_1;
    using std::placeholders::_2;

    // Set timeout waiting for GameLift websocket server to respond on initial connection.
    // See: https://github.com/zaphoyd/websocketpp/blob/master/websocketpp/connection.hpp#L501
    m_webSocketClient->set_open_handshake_timeout(WEBSOCKET_OPEN_HANDSHAKE_TIMEOUT_MILLIS);

    m_webSocketClient->set_tls_init_handler(std::bind(&WebSocketppClientWrapper::OnTlsInit, this, _1));
    m_webSocketClient->set_open_handler(std::bind(&WebSocketppClientWrapper::OnConnected, this, _1));
    m_webSocketClient->set_message_handler(std::bind(&WebSocketppClientWrapper::OnMessage, this, _1, _2));
    m_webSocketClient->set_fail_handler(std::bind(&WebSocketppClientWrapper::OnError, this, _1));
    m_webSocketClient->set_close_handler(std::bind(&WebSocketppClientWrapper::OnClose, this, _1));
}

WebSocketppClientWrapper::~WebSocketppClientWrapper() {
    // stop perpetual mode, allowing the websocketClient to destroy itself
    if (m_webSocketClient) {
        m_webSocketClient->stop_perpetual();
    }

    // close connections and join the thread
    if (m_connection && m_connection->get_state() == websocketpp::session::state::open) {
        Disconnect();
    }
    if (m_socket_thread_1 && m_socket_thread_1->joinable()) {
        m_socket_thread_1->join();
    }
    if (m_socket_thread_2 && m_socket_thread_2->joinable()) {
        m_socket_thread_2->join();
    }
}

GenericOutcome WebSocketppClientWrapper::Connect(const Uri &uri) {
    // Perform connection with retries.
    // This attempts to start up a new websocket connection / thread
    GeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder()
                                    .WithRetryStrategy(&retryStrategy)
                                    .WithCallable([this, &uri] {
                                        WebSocketppClientType::connection_ptr newConnection = PerformConnect(uri);
                                        if (newConnection && newConnection->get_state() == websocketpp::session::state::open) {
                                            // "Flip" traffic from our old websocket to our new websocket. Close the old one
                                            // if necessary
                                            WebSocketppClientType::connection_ptr oldConnection = m_connection;
                                            m_connection = newConnection;
                                            if (oldConnection && oldConnection->get_state() == websocketpp::session::state::open) {
                                                websocketpp::lib::error_code errorCode;
                                                m_webSocketClient->close(oldConnection->get_handle(), websocketpp::close::status::going_away,
                                                                         "Websocket client reconnecting", errorCode);
                                            }
                                            return true;
                                        } else {
                                            return false;
                                        }
                                    })
                                    .Build();
    callable.call();

    if (IsConnected()) {
        return GenericOutcome(nullptr);
    } else {
        return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE);
    }
}

WebSocketppClientType::connection_ptr WebSocketppClientWrapper::PerformConnect(const Uri &uri) {
    // Create connection request
    websocketpp::lib::error_code errorCode;
    WebSocketppClientType::connection_ptr newConnection = m_webSocketClient->get_connection(uri.GetUriString(), errorCode);
    if (errorCode.value()) {
        return newConnection;
    }

    // Queue a new connection request (the socket thread will act on it and attempt to connect)
    m_webSocketClient->connect(newConnection);

    // Wait for connection to succeed or fail (this makes connection synchronous)
    {
        std::unique_lock<std::mutex> lk(m_lock);
        m_cond.wait(lk, [this] { return m_connectionStateChanged; });
        m_connectionStateChanged = false;
    }

    return newConnection;
}

GenericOutcome WebSocketppClientWrapper::SendSocketMessage(const std::string &requestId, const std::string &message) {
    if (requestId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION));
    }

    std::future<GenericOutcome> responseFuture;
    // Lock whenever we make use of 'm_requestIdToPromise' to avoid concurrent writes/reads
    {
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        // This indicates we've already sent this message, and it's still in flight
        if (m_requestIdToPromise.count(requestId) > 0) {
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
        }

        std::promise<GenericOutcome> responsePromise;
        responseFuture = responsePromise.get_future();
        m_requestIdToPromise[requestId] = std::move(responsePromise);
    }

    GenericOutcome immediateResponse = SendSocketMessageAsync(message);

    if (!immediateResponse.IsSuccess()) {
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        m_requestIdToPromise.erase(requestId);
        return immediateResponse;
    }

    std::future_status promiseStatus = responseFuture.wait_for(std::chrono::milliseconds(SERVICE_CALL_TIMEOUT_MILLIS));

    if (promiseStatus == std::future_status::timeout) {
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        m_requestIdToPromise.erase(requestId);
        // If a call times out, retry
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE));
    }

    return responseFuture.get();
}

GenericOutcome WebSocketppClientWrapper::SendSocketMessageAsync(const std::string &message) {
    websocketpp::lib::error_code errorCode;
    m_webSocketClient->send(m_connection->get_handle(), message.c_str(), websocketpp::frame::opcode::text, errorCode);
    if (errorCode.value()) {
        if (errorCode.value() == websocketpp::error::no_outgoing_buffers) {
            // If buffers are full, send will fail. Retryable since buffers can free up as messages
            // send.
            return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE);
        }
        return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_SEND_MESSAGE_FAILURE);
    }
    return GenericOutcome(nullptr);
}

void WebSocketppClientWrapper::Disconnect() {
    if (m_connection != nullptr) {
        websocketpp::lib::error_code ec;
        m_webSocketClient->close(m_connection->get_handle(), websocketpp::close::status::going_away, "Websocket client closing", ec);
        m_connection = nullptr;
    }
}

void WebSocketppClientWrapper::RegisterGameLiftCallback(const std::string &gameLiftEvent, const std::function<GenericOutcome(std::string)> &callback) {
    m_eventHandlers[gameLiftEvent] = callback;
}

bool WebSocketppClientWrapper::IsConnected() {
    // m_connection is nullptr if 'm_webSocketClient->get_connection()' fails
    return m_connection != nullptr && m_connection->get_state() == websocketpp::session::state::open;
}

void WebSocketppClientWrapper::OnConnected(websocketpp::connection_hdl connection) {
    // aquire lock and set condition variables (let main thread know connection is successful)
    {
        std::lock_guard<std::mutex> lk(m_lock);
        // set the state change variables and notify the thread that is connecting
        m_connectionStateChanged = true;
    }
    m_cond.notify_one();
}

void WebSocketppClientWrapper::OnError(websocketpp::connection_hdl connection) {
    auto con = m_webSocketClient->get_con_from_hdl(connection);

    // aquire lock and set condition variables (let main thread know an error has occurred)
    {
        std::lock_guard<std::mutex> lk(m_lock);
        // set the state change variables and notify the thread that is connecting
        m_connectionStateChanged = true;
    }
    m_cond.notify_one();
}

void WebSocketppClientWrapper::OnMessage(websocketpp::connection_hdl connection, websocketpp::config::asio_client::message_type::ptr msg) {
    std::string message = msg->get_payload();

    ResponseMessage responseMessage;
    Message &gameLiftMessage = responseMessage;
    if (!gameLiftMessage.Deserialize(message)) {
        return;
    }

    const std::string &action = responseMessage.GetAction();
    const std::string &requestId = responseMessage.GetRequestId();
    const int statusCode = responseMessage.GetStatusCode();
    const std::string &errorMessage = responseMessage.GetErrorMessage();

    // Default to a success response with no result pointer
    GenericOutcome response(nullptr);
    // Check if the response was an error. If so, update the response based on status code.
    // RequestId will be empty when we get a message not associated with a request, in which case we
    // don't expect a 200 status code either.
    if (statusCode != OK_STATUS_CODE && !requestId.empty()) {
        response = GenericOutcome(GameLiftError(statusCode, message.c_str()));
    } else {
        // If we got a success response, and we have a special event handler for this action, invoke
        // it to get the real parsed result
        if (m_eventHandlers.count(action)) {
            response = m_eventHandlers[action](message);
        }
    }

    // Lock whenever we make use of 'm_requestIdToPromise' to avoid concurrent writes/reads
    std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
    if (m_requestIdToPromise.count(requestId) > 0) {
        m_requestIdToPromise[requestId].set_value(response);
        m_requestIdToPromise.erase(requestId);
    }
}

websocketpp::lib::shared_ptr<asio::ssl::context> WebSocketppClientWrapper::OnTlsInit(websocketpp::connection_hdl hdl) {
    websocketpp::lib::shared_ptr<asio::ssl::context> contextPtr(new asio::ssl::context(asio::ssl::context::tlsv12));
    return contextPtr;
}

void WebSocketppClientWrapper::OnClose(websocketpp::connection_hdl connection) {}

} // namespace Internal
} // namespace GameLift
} // namespace Aws