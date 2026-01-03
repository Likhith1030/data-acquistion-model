# Scientific Instrument Data Acquisition Module

A C++ library for capturing, buffering, and processing high-throughput data streams from scientific instrumentation.

## Features

- **Lock-free ring buffer** — SPSC ring buffer for zero-allocation, low-latency sample queuing
- **DataStream** — threaded capture loop with configurable sample rate, per-stream overflow tracking, and optional callbacks
- **DataProcessor** — composable pipeline stages (range filter, moving average, gain) with Welford online statistics
- **Instrument abstraction** — pluggable backend interface; ships with a `SimulatedInstrument` for testing
- **PerformanceProfiler** — nanosecond-resolution `ScopedTimer` with mean / p50 / p95 / p99 latency reporting

## Project structure

```
.
├── include/          # Public headers
│   ├── RingBuffer.h
│   ├── DataSample.h
│   ├── DataStream.h
│   ├── DataProcessor.h
│   ├── Instrument.h
│   └── PerformanceProfiler.h
├── src/              # Implementation files
├── tests/            # Google Test unit tests
├── main.cpp          # Demo application
└── CMakeLists.txt
```

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Run the demo

```bash
./build/daq_demo
```

### Run unit tests

```bash
cd build && ctest --output-on-failure
```

## Technologies

C++ 17 · CMake · Google Test · pthreads · Linux · Performance Profiling · Scientific / Analytical Instrumentation
