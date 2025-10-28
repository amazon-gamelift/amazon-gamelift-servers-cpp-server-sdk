- [C++ Server SDK for Amazon GameLift Servers Metrics API](#c-server-sdk-for-amazon-gamelift-servers-metrics-api)
  - [Metrics Setup & Workflow](#metrics-setup--workflow)
    - [Step 1: Preresiquites](#step-1-preresiquites)
    - [Step 2: Declare and Use Custom Metrics](#step-2-declare-and-use-custom-metrics)
    - [Step 3: Process Metrics Regularly](#step-3-process-metrics-regularly)
    - [Global tags](#global-tags)
  - [Metrics Declaration & Definition](#metrics-declaration--definition)
    - [Defining Platforms](#defining-platforms)
      - [Define Platform](#define-platform)
      - [Define Platform as API](#define-platform-as-api)
    - [Declaring & Defining Metrics](#declaring--defining-metrics)
      - [Declare Metrics](#declare-metrics)
      - [Declare Metrics as API](#declare-metrics-as-api)
      - [Define Metrics](#define-metrics)
  - [Metrics Usage & Operations](#metrics-usage--operations)
    - [Gauges](#gauges)
    - [Counters](#counters)
    - [Timers](#timers)
    - [Samplers](#samplers)
    - [Derived Metrics](#derived-metrics)
    - [Tagging](#tagging)
  - [Thread Safety Note](#thread-safety-note)


# C++ Server SDK for Amazon GameLift Servers Metrics API

The C++ server SDK for Amazon GameLift Servers provides a comprehensive metrics system for collecting and sending custom
metrics from your game servers hosted on Amazon GameLift Server. These metrics can be integrated with various visualization
and aggregation tools including Amazon Managed Grafana, Amazon Managed Prometheus, Amazon CloudWatch, and other monitoring platforms.
This documentation provides explanations and instructions for advanced configuration and custom metrics implementation.
For a quick start and workflow setup, refer to [METRICS.md](METRICS.md).

The C++ API can be accessed by including the `aws/gamelift/metrics/GameLiftMetrics.h` header.

## Metrics Setup & Workflow

Follow these steps to set up and initialize the metrics system in your game server. These metrics functions are in the
`Aws::GameLift::Metrics namespace`. For detailed API reference, see [GlobalMetricsProcessor.h](../gamelift-server-sdk/include/aws/gamelift/metrics/GlobalMetricsProcessor.h).

### Step 1: Preresiquites

Complete the Step 1 from [METRICS.md](METRICS.md#step-1-integrate-metrics-feature-into-your-game-project).

### Step 2: Declare and Use Custom Metrics

Define platforms and declare your custom metrics:

```cpp
// Define platform for metrics sending
GAMELIFT_METRICS_DEFINE_PLATFORM(ServerRelease, 1);

// Declare custom metrics
GAMELIFT_METRICS_DECLARE_GAUGE(ServerPlayers, "server_players", ServerRelease,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ServerTickTime, "server_tick_time", ServerRelease,
                               Aws::GameLift::Metrics::SampleAll());

// Define metrics in source file
GAMELIFT_METRICS_DEFINE_GAUGE(ServerPlayers);
GAMELIFT_METRICS_DEFINE_TIMER(ServerTickTime);

// Use metrics in the game loop
GAMELIFT_METRICS_INCREMENT(ServerPlayers);
GAMELIFT_METRICS_TIME_SCOPE(ServerTickTime);
```

### Step 3: Process Metrics Regularly

Call `MetricsProcess()` periodically (typically in your main game loop) to ensure metrics are sent via the configured callback:

```cpp
void gameLoop() {
    while (gameRunning) {
        // Game logic here

        // Process and send metrics
        Aws::GameLift::Metrics::MetricsProcess();

        // Sleep or wait for next frame
    }
}
```

### Global tags

These tags are automatically attached to all metrics during initialization.

| Tag key               | Description                                      |
|:----------------------|:-------------------------------------------------|
| `gamelift_process_id` | Amazon GameLift Servers managed process ID.      |
| `process_pid`         | OS process ID.                                   |
| `session_id`          | Amazon GameLift Servers managed game session ID. |



## Metrics Declaration & Definition
### Defining Platforms

Platforms enable conditional compilation of metrics based on build configuration. They use compile-time evaluation to determine
whether metrics should be active and when a particular metric should be used.

#### Define Platform

Creates a platform struct, this can be placed in a header or source file.

```c
GAMELIFT_METRICS_DEFINE_PLATFORM(<Platform>, <Condition>);
```

Parameters:
- `<Platform>`: Name of the platform struct (e.g., Server, ServerDebug). To be used when declaring metrics.
- `<Condition>`: Compile-time boolean expression or integer condition.

#### Define Platform as API

Same as `DEFINE_PLATFORM` but adds API export decoration for cross-module usage. It can be placed in a header or source file.

```c
GAMELIFT_METRICS_DEFINE_PLATFORM_API(<API>, <Platform>, <Condition>);
```

Parameters:
- `<API>`: Export macro (e.g., `GAMEMODULE_API` for Unreal Engine DLL export)
- `<Platform>`: Platform struct name. To be used when declaring metrics.
- `<Condition>`: Compile-time boolean expression or integer condition.

### Declaring & Defining Metrics

Metrics follow a two-phase definition pattern: declaration (creates the class interface) and definition (implements the
singleton instance). This separation allows header-only declarations with single-source-file definitions.

#### Declare Metrics
Creates a metric class. The class includes platform checking, sampling logic, and derived metrics support.
It can be placed in a header or source file.

```c
GAMELIFT_METRICS_DECLARE_GAUGE(<Metric Class>, <Key>, <Platform>, <Sampler>);
GAMELIFT_METRICS_DECLARE_TIMER(<Metric Class>, <Key>, <Platform>, <Sampler>);
GAMELIFT_METRICS_DECLARE_COUNTER(<Metric Class>, <Key>, <Platform>, <Sampler>);
```

> NOTE: All declaration macros can optionally take a list of derived metrics as a final parameter. See the Derived Metrics section below for more information.

Parameters:
- `<Metric Class>`: Class name defined by the macro (e.g., ServerPlayers, ServerConnections).
- `<Key>`: String literal for StatsD metric name (e.g., "server_players", "server_connections").
- `<Platform>`: Platform defined with `GAMELIFT_METRICS_DEFINE_PLATFORM`.
- `<Sampler>`: Sampling strategy instance (e.g., `Aws::GameLift::Metrics::SampleAll()`)
  - We provide default `SampleAll()` and `SampleFraction(Fraction)`samplers.
  - Custom samplers may be defined by the user.

#### Declare Metrics as API

Identical to regular declaration but adds API export decoration for cross-module visibility.

```c
GAMELIFT_METRICS_DECLARE_GAUGE_API(<API>, <Metric Class>, <Key>, <Platform>, <Sampler>);
GAMELIFT_METRICS_DECLARE_TIMER_API(<API>, <Metric Class>, <Key>, <Platform>, <Sampler>);
GAMELIFT_METRICS_DECLARE_COUNTER_API(<API>, <Metric Class>, <Key>, <Platform>, <Sampler>);
```

Parameters:
- `<API>`: Export macro (e.g., `GAMEMODULE_API` for Unreal Engine DLL export)
  - Technical details:
    - On Windows `MODULENAME_API` expands into `__declspec(dllexport)` in module source files and `__declspec(dllimport)` in headers and other modules. This exposes a class across DLL boundaries.
    - On Linux it expands into nothing.
  - This enables developers to declare metrics to be used by other modules.
- The rest of the arguments have the same meaning as in the previous section.

#### Define Metrics

Defines a metric class. Must be placed in a single source file.

```c
GAMELIFT_METRICS_DEFINE_GAUGE(<Metric Class>);
GAMELIFT_METRICS_DEFINE_COUNTER(<Metric Class>);
GAMELIFT_METRICS_DEFINE_TIMER(<Metric Class>);
```

Parameters:
- `<Metric Class>`: The name of the class declared by the `GAMELIFT_METRICS_DECLARE` macro.

Important Notes:
- Each metric must be defined exactly once across all source files.
- Definition must occur after metric declaration.
- Linking errors may occur if definition is missing or duplicated.


## Metrics Usage & Operations
### Gauges

Gauges represent metrics that track the current value of something over time. They maintain state and are ideal for
measurements like player count, memory usage, connection count, or any value that can go up and down.

#### Declaring Gauges

First declare the gauge using `GAMELIFT_METRICS_DECLARE_GAUGE`, then define it with `GAMELIFT_METRICS_DEFINE_GAUGE`.

Example:
```c
// Header file
GAMELIFT_METRICS_DECLARE_GAUGE(ServerPlayers, "server_players", ServerMetrics, Aws::GameLift::Metrics::SampleAll());

// Source file  
GAMELIFT_METRICS_DEFINE_GAUGE(ServerPlayers);
```

#### Set Gauge

Sets a gauge to an absolute value. Always executes the expression and returns the evaluated value.

```c
GAMELIFT_METRICS_SET(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.
- `<Value>`: Expression evaluating to numeric value (int, float).

#### Set Gauge (Sampled)

Sets a gauge to an absolute value. Only evaluates the expression if the sampler decides to take the sample. Does not return anything.

```c
GAMELIFT_METRICS_SET_SAMPLED(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.
- `<Value>`: Expression evaluating to numeric value (int, float).


#### Arithmetic Operations

Modify gauge values relative to current state. ADD/SUBTRACT work by sending delta values to the metrics processor, which maintains the running total.

```c
GAMELIFT_METRICS_ADD(<Metric Class>, <Value>);               // Always executes expression
GAMELIFT_METRICS_SUBTRACT(<Metric Class>, <Value>);          // Always executes expression
GAMELIFT_METRICS_ADD_SAMPLED(<Metric Class>, <Value>);       // Only executes if sampled
GAMELIFT_METRICS_SUBTRACT_SAMPLED(<Metric Class>, <Value>);  // Only executes if sampled
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.
- `<Value>`: Expression evaluating to numeric value (int, float).

#### Increment / Decrement Operations

Increment or decrement a gauge by 1.

```c
GAMELIFT_METRICS_INCREMENT(<Metric Class>);
GAMELIFT_METRICS_DECREMENT(<Metric Class>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.


#### Reset Gauge

Set a gauge to 0.

```c
GAMELIFT_METRICS_RESET(<Metric Class>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.


#### Counting Operations

Semantic aliases for gauge increment operations with specific use cases. (Alias for `_INCREMENT` with semantic meaning.)

```c
GAMELIFT_METRICS_COUNT_HIT(<Metric Class>);                    // Alias for INCREMENT
GAMELIFT_METRICS_COUNT_EXPR(<Metric Class>, <Expression>);     // Increment then return expression result
```
COUNT_HIT: Used for counting events, line hits, or occurrences. Semantically identical to INCREMENT.
COUNT_EXPR: Increments the gauge and then evaluates and returns the expression result.

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_GAUGE` macro.
- `<Expression>`: C++ expression that will be evaluated and its result returned by the macro.



### Counters

Counters represent metrics that track cumulative occurrences over time. Unlike gauges, counters only increase and are ideal for measuring
events like bytes sent, packets received, function calls, or any event that happens repeatedly. Counters accumulate values and never decrease.

#### Declaring Counters

First declare the counter using `GAMELIFT_METRICS_DECLARE_COUNTER`, then define it with `GAMELIFT_METRICS_DEFINE_COUNTER`.

Example:
```c
// Header file
GAMELIFT_METRICS_DECLARE_COUNTER(ServerBytesIn, "server_bytes_in", ServerMetrics, Aws::GameLift::Metrics::SampleAll());

// Source file  
GAMELIFT_METRICS_DEFINE_COUNTER(ServerBytesIn);

```

#### Add to Counter

Adds a value to a counter. Always executes the expression.

```c
GAMELIFT_METRICS_ADD(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_COUNTER` macro.
- `<Value>`: Expression evaluating to a positive numeric value (int, float).

#### Add to Counter (Sampled)

Adds a value to a counter. Only evaluates the expression if the sampler decides to take the sample.

```c
GAMELIFT_METRICS_ADD_SAMPLED(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_COUNTER` macro.
- `<Value>`: Expression evaluating to a positive numeric value (int, float).

#### Increment Counter

Increments a counter by 1.

```c
GAMELIFT_METRICS_INCREMENT(<Metric Class>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_COUNTER` macro.

#### Counting Operations

Semantic aliases for counter increment operations with specific use cases. (Alias for `_INCREMENT` with semantic meaning.)

```c
GAMELIFT_METRICS_COUNT_HIT(<Metric Class>);                    // Alias for INCREMENT
GAMELIFT_METRICS_COUNT_EXPR(<Metric Class>, <Expression>);     // Increment then return expression result
```
COUNT_HIT: Increments a counter by 1. Semantic alias for INCREMENT used for counting code execution.
COUNT_EXPR: Increments the counter and then evaluates and returns the expression result.

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_COUNTER` macro.
- `<Expression>`: C++ expression that will be evaluated and its result returned by the macro.


### Timers

Timers represent duration measurements. They are ideal for tracking execution time, session duration, response times,
or any time-based metrics. Timers support derived metrics like mean, percentiles, and latest values for statistical analysis.

#### Declaring Timers

First declare the timer using `GAMELIFT_METRICS_DECLARE_TIMER`, then define it with `GAMELIFT_METRICS_DEFINE_TIMER`.

Example:
```c
// Header file
GAMELIFT_METRICS_DECLARE_TIMER(ServerTickTime, "server_tick_time", ServerMetrics, Aws::GameLift::Metrics::SampleAll());

// Source file  
GAMELIFT_METRICS_DEFINE_TIMER(ServerTickTime);
```

#### Set Milliseconds

Sets a timer to a duration value in milliseconds. Always executes the expression and returns the evaluated value.

```c
GAMELIFT_METRICS_SET_MS(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.
- `<Value>`: Expression evaluating to milliseconds (int, float).

#### Set Seconds

Sets a timer to a duration value in seconds. Always executes the expression and returns the evaluated value.

```c
GAMELIFT_METRICS_SET_SEC(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.
- `<Value>`: Expression evaluating to seconds (int, float).


#### Set Milliseconds (Sampled)

Sets a timer to a duration value in milliseconds. Only evaluates the expression if the sampler decides to take the sample. Does not return anything.

```c
GAMELIFT_METRICS_SET_MS_SAMPLED(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.
- `<Value>`: Expression evaluating to seconds (int, float).

#### Set Seconds (Sampled)

Sets a timer to a duration value in seconds. Only evaluates the expression if the sampler decides to take the sample. Does not return anything.

```c
GAMELIFT_METRICS_SET_SEC_SAMPLED(<Metric Class>, <Value>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.
- `<Value>`: Expression evaluating to seconds (int, float).

#### Time Expression

Times the execution duration of an expression.

```c
GAMELIFT_METRICS_TIME_EXPR(<Metric Class>, <Expression>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.
- `<Expression>`: C++ expression that will be evaluated and its result returned by the macro.

#### Time Scope

Times the scope starting at the call site and ending when the surrounding scope exits. (Block reaches end or function returns early.)

```c
GAMELIFT_METRICS_TIME_SCOPE(<Metric Class>);
```

Parameters:
- `<Metric Class>`: Name of the class declared by the `GAMELIFT_METRICS_DECLARE_TIMER` macro.


#### ScopedTimer Class

A `ScopedTimer<M>` class is defined in `ScopedTimer.h` and acts as a RAII-style timer.

It takes the timer class `M` declared by `GAMELIFT_METRICS_DECLARE_TIMER` as type parameter. The timing begins when `ScopedTimer` is constructed and ends when it is destroyed.


### Samplers

Samplers control which metric samples are recorded, enabling performance optimization and data volume management. They are
specified when declaring metrics and determine whether each metric operation should be processed or ignored.

#### SampleAll

Records every sample. No filtering is applied.

```c
SampleAll()
```

#### SampleNone

Records no samples. Convenient for temporarily disabling metrics without changing code.

```c
SampleNone()
```

#### SampleFraction

Records a random fraction of all samples using per-thread random number generators.

**Default seed (current time):**
```c
SampleFraction(<Fraction>)
```

Parameters:
- `<Fraction>`: A fraction of values to sample. For example, `0.1` would sample `10%` of the time.

**Custom seed:**
```c
SampleFraction(<Fraction>, <Seed>)
```

Parameters:
- `<Fraction>`: A fraction of values to sample. For example, `0.1` would sample `10%` of the time.
- `<Seed>`: 64-bit integer seed for the random number generator

#### Custom Samplers
Implement the `ISampler` interface to create custom sampling logic. See `Samplers.h` for examples.

```c
struct GAMELIFT_METRICS_API ISampler {
  virtual ~ISampler();

  /**
   * Checks if we want to take this sample and updates internal state (if any).
   * @return true if this sample should be recorded, false otherwise
   */
  virtual bool ShouldTakeSample() = 0;

  /**
   * Gets the sample rate for this sampler
   * @return The sample rate as a float between 0.0 and 1.0 inclusive
   */
  virtual float GetSampleRate() const = 0;
};
```

> Note: `ShouldTakeSample` must be a thread-safe function. It may be called from multiple threads. As such, the implementation must be thread-safe via synchronization primitives or an asynchronous solution.


### Derived Metrics

Derived metrics compute additional statistics from base metrics automatically during each capture period. They enable statistical
analysis without requiring separate metric declarations. Derived metrics are specified as optional parameters when declaring gauges
and timers (counters do not support derived metrics).

```cpp
GAMELIFT_METRICS_DECLARE_TIMER(
    MetricGameDeltaTime,
    "server_delta_time",
    Aws::GameLift::Metrics::Server,
    Aws::GameLift::Metrics::SampleAll(),
    Aws::GameLift::Metrics::Percentiles(0.5, 0.9, 0.95)
);
```

A number of derived metrics classes are included by default. The user may define custom derived metrics by inheriting the `IDerivedMetric` interface.

#### Min, Max

Computes the minimum or maximum value. For example, the highest value a gauge was set to or the longest timing of a timer.

```c
Min()
```

```c
Max()
```

The derived metric is logged with a `.min` and `.max` key suffix respectively. For example, a `player_count` gauge would have a `player_count.max` value if `Max()` derived metric is added.

#### Mean

Computes the arithmetic average of all values recorded during the capture period.

```c
Mean()
```

The derived metric is logged with a `.mean` key suffix. For example, a `player_count` gauge would have a `player_count.mean` value if `Mean()` derived metric is added.


```c
Mean(const char* KeySuffix)
```

Parameters:
- `KeySuffix`: A custom suffix to append to the original metric’s key.

The derived metric is logged with a user specified suffix. For example, a `player_count` gauge would have a `player_count.foo` value if `Mean(".foo")` derived metric is added.

#### Percentiles

Computes specified percentiles from all values recorded during the capture period.

```c
Percentiles(double Percentile1, double Percentile2, ..., double PercentileN)
```

Parameters:
- `Percentile1` to `PercentileN` is a list of percentile boundaries in the `0` to `1` range.

For example, `Percentiles(0.1, 0.5, 0.8, 0.9, 0.95)` computes the 10th, 50th, 80th, 90th and 95th percentile of the parent metric.

The percentiles are then logged with a `.pXX` suffix. For example, if applied to `tick_time`, this would record the following metrics: `tick_time`, `tick_time.p10`, `tick_time.p50`, `tick_time.p80`, `tick_time.p90`, and `tick_time.p95`.

#### Median

Computes the median.

```c
Median()
```

Records median with the `.p50` (50-th percentile) suffix.

#### Latest

Keeps only the latest seen value.

```c
Latest()
```

The derived metric is logged with a `.latest` key suffix. For example, a `tick_time` timer would have a `tick_time.latest` value if `Latest()` derived metric is added.

```c
Latest(const char* KeySuffix)
```

The derived metric is logged with a user specified suffix. For example, a `tick_time` timer would have a `tick_time.foo` value if `Latest(“.foo”)` derived metric is added.

#### Reduce

Applies a reduction to recorded values.

Takes an initial value and a user specified operation to apply to every sample.

```c
Reduce<Operation>(const char* KeySuffix)
```
Note:
- `Operation`: A type implementing the C++ operation pattern. It is default constructible and implements `operator()(double Current, double New)` where `Current` is the current reduced value and `New` is the newly recorded sample.

Parameters:
- `KeySuffix`: Custom suffix to append to the original metric’s key.

#### Reduce with Initial Value

```c
Reduce<Operation>(const char* KeySuffix, double InitialValue)
```
Note:
- `Operation`: A type implementing the C++ operation pattern. It is default constructible and implements `operator()(double Current, double New)` where `Current` is the current reduced value and `New` is the newly recorded sample.

- Parameters:
- `KeySuffix`: Custom suffix to append to the original metric’s key.
- `InitialValue`: Initial value to use for the reduction.


```c
Reduce(const char* KeySuffix, double InitialValue, auto OperationFunc)
```
Note:
- `Operation`: A type implementing the C++ operation pattern. It is default constructible and implements `operator()(double Current, double New)` where `Current` is the current reduced value and `New` is the newly recorded sample.

- Parameters:
- `KeySuffix`: Custom suffix to append to the original metric’s key.
- `InitialValue`: Initial value to use for the reduction.
- `OperationFunc`: A callable value. For example, a function pointer, or lambda. It must be callable as `double OperationFunc(double Current, double New)` where `Current` is the current reduced value and `New` is the newly recorded sample. The return value is the new reduced value.


#### Custom Derived Metrics

A simple way to define a custom derived metric is to inherit the `Reduce` class with a custom operation. For examples of this, see `ReduceMetric.h` where we define `Reduce`, `Min` and `Max`. Also see `Latest.h` and `Mean.h`.

For complex statistics, implement the `IDerivedMetric` interface:

```c
struct GAMELIFT_METRICS_API IDerivedMetric {
  virtual ~IDerivedMetric() = default;

  /**
   * Handles metric messages.
   *
   * This is the place to intercept them and update internal state.
   */
  virtual void HandleMessage(MetricMessage &Message,
                             IMetricsEnqueuer &Submitter) = 0;

  /**
   * Emits metrics before sending.
   *
   * Called once per parent metric per capture period.
   */
  virtual void EmitMetrics(const IMetric *OriginalMetric,
                           IMetricsEnqueuer &Submitter) = 0;
};
```

- `HandleMessage` called for each sample during capture period.
- `EmitMetrics` called once at end of capture period.

See `ReduceMetric.h` or `Percentiles.h` for sample implementations.

Additionally, a couple helper classes are defined:
- `KeySuffix` in `KeySuffix.h`: Provides cross-platform string handling for metric key suffixes.
- `DynamicMetric` in `DynamicMetric.h`: Creates metric instances dynamically for derived metrics.

### Tagging

Metrics can be tagged with key-value pairs to attach additional contextual information for filtering and grouping in monitoring tools
like Grafana, CloudWatch, or Prometheus. Tags are applied when metrics are sent to the collector and become part of the metric's identity.

Two types of tags are available: **global tags** (applied to all metrics) and **per-metric tags** (applied to specific metrics).
Global tags are used internally for session IDs and process IDs, while per-metric tags enable custom categorization.

Tags are applied prior to the SDK sending the stats to a collector process. Any tags active at that time are applied.

#### Set Global Tag

Sets or updates a global tag that applies to all metrics.

```c
GAMELIFT_METRICS_GLOBAL_TAG_SET(<Platform>, <Key>, <Value>);
```

Parameters:
- `<Platform>`: Platform class defined with `GAMELIFT_METRICS_DEFINE_PLATFORM`. The tag will not be applied unless the platform is active.
- `<Key>`: UTF-8 string for the tag name (const char* or std::string).
- `<Value>`: UTF-8 string for the tag value (const char* or std::string).

`<Key>` and `<Value>` are both copied and can be freed once the function exits.

#### Remove Global Tag

Removes a global tag by key.

```c
GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(<Platform>, <Key>);
```

Parameters:
- `<Platform>`: Platform class defined with `GAMELIFT_METRICS_DEFINE_PLATFORM`. The tag will not be applied unless the platform is active.
- `<Key>`: UTF-8 string for the tag name (const char* or std::string).

`<Key>` is copied and can be freed once the function exits.

#### Set Per-Metric Tag

Sets or updates a tag for a specific metric.

```c
GAMELIFT_METRICS_TAG_SET(<Metric Class>, <Key>, <Value>);
```

Parameters:
- `<Platform>`: Platform class defined with `GAMELIFT_METRICS_DEFINE_PLATFORM`. The tag will not be applied unless the platform is active.
- `<Key>`: UTF-8 string for the tag name (const char* or std::string).
- `<Value>`: UTF-8 string for the tag value (const char* or std::string).

`<Key>` and `<Value>` are both copied and can be freed once the function exits.

#### Remove Per-Metric Tag

Removes a tag from a specific metric.

```c
GAMELIFT_METRICS_TAG_REMOVE(<Metric Class>, <Key>);
```

Parameters:
- `<Platform>`: Platform class defined with `GAMELIFT_METRICS_DEFINE_PLATFORM`. The tag will not be applied unless the platform is active.
- `<Key>`: UTF-8 string for the tag name (const char* or std::string).

`<Key>` is copied and can be freed once the function exits.

## Thread Safety Note

The C++ Metrics SDK is fully thread-safe. Metrics may be logged by multiple producer threads simultaneously. `MetricsProcess()`
may be called from any thread at any time as well. The logging is asynchronous, the thread running `MetricsProcess()` will not
block producing threads. Instead, metrics logged during processing may miss the current capture period and be included in the
next one. Metrics are never lost (a Multi-Producer, Single-Consumer queue  is used internally).

The only thread-safety concern is race conditions when writing to the same metric from multiple threads simultaneously. For example, if setting
a gauge from two threads, one thread will submit its value last and overwrite the previous one. The order is non-deterministic. This affects
timers and gauges. Counters are not affected since all additions are summed when the metrics are processed.

This scenario is uncommon and the behavior is expected. It is the user's responsibility to provide external synchronization if deterministic
ordering is required for the same metric across multiple threads.
