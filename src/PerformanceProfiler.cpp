#include "PerformanceProfiler.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace daq {

void PerformanceProfiler::record(const std::string& label, Duration elapsed) {
    const double us = static_cast<double>(elapsed.count()) / 1000.0;
    measurements_us_[label].push_back(us);
}

LatencyStats PerformanceProfiler::stats(const std::string& label) const {
    auto it = measurements_us_.find(label);
    if (it == measurements_us_.end() || it->second.empty())
        return {};

    std::vector<double> sorted = it->second;
    std::sort(sorted.begin(), sorted.end());

    const std::size_t n = sorted.size();
    LatencyStats s;
    s.sample_count = n;
    s.min_us       = sorted.front();
    s.max_us       = sorted.back();
    s.mean_us      = std::accumulate(sorted.begin(), sorted.end(), 0.0) / n;
    s.p50_us       = sorted[n / 2];
    s.p95_us       = sorted[static_cast<std::size_t>(n * 0.95)];
    s.p99_us       = sorted[static_cast<std::size_t>(n * 0.99)];
    return s;
}

void PerformanceProfiler::reset(const std::string& label) {
    measurements_us_.erase(label);
}

void PerformanceProfiler::reset_all() {
    measurements_us_.clear();
}

std::vector<std::string> PerformanceProfiler::labels() const {
    std::vector<std::string> out;
    out.reserve(measurements_us_.size());
    for (const auto& [k, _] : measurements_us_)
        out.push_back(k);
    return out;
}

void PerformanceProfiler::print_report() const {
    std::cout << std::left
              << std::setw(30) << "Label"
              << std::setw(10) << "N"
              << std::setw(12) << "Mean(us)"
              << std::setw(12) << "P50(us)"
              << std::setw(12) << "P95(us)"
              << std::setw(12) << "P99(us)"
              << std::setw(12) << "Min(us)"
              << std::setw(12) << "Max(us)"
              << '\n';
    std::cout << std::string(110, '-') << '\n';

    for (const auto& [label, _] : measurements_us_) {
        const auto s = stats(label);
        std::cout << std::left
                  << std::setw(30) << label
                  << std::setw(10) << s.sample_count
                  << std::setw(12) << std::fixed << std::setprecision(2) << s.mean_us
                  << std::setw(12) << s.p50_us
                  << std::setw(12) << s.p95_us
                  << std::setw(12) << s.p99_us
                  << std::setw(12) << s.min_us
                  << std::setw(12) << s.max_us
                  << '\n';
    }
}

} // namespace daq
