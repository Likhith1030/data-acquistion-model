#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace daq {

struct LatencyStats {
    double mean_us{0.0};
    double min_us{0.0};
    double max_us{0.0};
    double p50_us{0.0};
    double p95_us{0.0};
    double p99_us{0.0};
    std::uint64_t sample_count{0};
};

class ScopedTimer;

class PerformanceProfiler {
public:
    using Clock    = std::chrono::steady_clock;
    using Duration = std::chrono::nanoseconds;

    void record(const std::string& label, Duration elapsed);

    LatencyStats stats(const std::string& label) const;
    void         reset(const std::string& label);
    void         reset_all();

    std::vector<std::string> labels() const;

    void print_report() const;

private:
    std::unordered_map<std::string, std::vector<double>> measurements_us_;
};

// RAII timer: records elapsed time into the profiler on destruction.
class ScopedTimer {
public:
    ScopedTimer(PerformanceProfiler& profiler, std::string label)
        : profiler_(profiler), label_(std::move(label)),
          start_(PerformanceProfiler::Clock::now()) {}

    ~ScopedTimer() {
        auto elapsed = PerformanceProfiler::Clock::now() - start_;
        profiler_.record(label_, elapsed);
    }

private:
    PerformanceProfiler& profiler_;
    std::string          label_;
    PerformanceProfiler::Clock::time_point start_;
};

} // namespace daq
