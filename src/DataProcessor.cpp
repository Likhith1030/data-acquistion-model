#include "DataProcessor.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <memory>

namespace daq {

void DataProcessor::add_stage(ProcessingStage stage) {
    stages_.emplace_back(std::move(stage));
}

void DataProcessor::clear_stages() {
    stages_.clear();
}

bool DataProcessor::process(DataSample& sample) {
    for (auto& stage : stages_) {
        if (!stage(sample)) {
            ++stats_.samples_rejected;
            return false;
        }
    }
    update_stats(sample);
    ++stats_.samples_processed;
    return true;
}

std::size_t DataProcessor::process_batch(std::vector<DataSample>& samples,
                                         std::vector<DataSample>& out) {
    out.clear();
    out.reserve(samples.size());
    for (auto& s : samples) {
        if (process(s))
            out.push_back(s);
    }
    return out.size();
}

void DataProcessor::reset_stats() {
    stats_     = {};
    welf_mean_ = 0.0;
    welf_m2_   = 0.0;
}

ProcessingStage DataProcessor::make_range_filter(std::size_t channel, double lo, double hi) {
    return [=](DataSample& s) -> bool {
        if (channel >= s.channel_count) return true;
        const double v = s.channels[channel];
        return v >= lo && v <= hi;
    };
}

ProcessingStage DataProcessor::make_moving_average(std::size_t channel, std::size_t window) {
    auto buf = std::make_shared<std::deque<double>>();
    auto sum = std::make_shared<double>(0.0);
    return [=](DataSample& s) mutable -> bool {
        if (channel >= s.channel_count) return true;
        buf->push_back(s.channels[channel]);
        *sum += s.channels[channel];
        if (buf->size() > window) {
            *sum -= buf->front();
            buf->pop_front();
        }
        s.channels[channel] = *sum / static_cast<double>(buf->size());
        return true;
    };
}

ProcessingStage DataProcessor::make_gain(std::size_t channel, double gain) {
    return [=](DataSample& s) -> bool {
        if (channel >= s.channel_count) return true;
        s.channels[channel] *= gain;
        return true;
    };
}

void DataProcessor::update_stats(const DataSample& s) {
    if (s.channel_count == 0) return;
    const double v = s.channels[0];

    // Welford online algorithm for mean and variance
    const double n    = static_cast<double>(stats_.samples_processed + 1);
    const double delta = v - welf_mean_;
    welf_mean_ += delta / n;
    welf_m2_   += delta * (v - welf_mean_);

    stats_.mean     = welf_mean_;
    stats_.variance = (n > 1) ? (welf_m2_ / (n - 1)) : 0.0;

    if (stats_.samples_processed == 0) {
        stats_.min_value = v;
        stats_.max_value = v;
    } else {
        stats_.min_value = std::min(stats_.min_value, v);
        stats_.max_value = std::max(stats_.max_value, v);
    }
}

} // namespace daq
