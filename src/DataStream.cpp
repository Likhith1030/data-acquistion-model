#include "DataStream.h"

#include <chrono>
#include <stdexcept>
#include <thread>

namespace daq {

DataStream::DataStream(StreamConfig cfg)
    : cfg_(std::move(cfg)), ring_(cfg_.buffer_capacity) {
    if (cfg_.sample_rate_hz == 0)
        throw std::invalid_argument("sample_rate_hz must be > 0");
    if (cfg_.channel_count == 0 || cfg_.channel_count > MAX_CHANNELS)
        throw std::invalid_argument("channel_count out of range");
}

DataStream::~DataStream() {
    stop();
}

void DataStream::start() {
    if (running_.exchange(true))
        return;
    capture_thread_ = std::thread(&DataStream::capture_loop, this);
}

void DataStream::stop() {
    if (!running_.exchange(false))
        return;
    if (capture_thread_.joinable())
        capture_thread_.join();
}

void DataStream::set_callback(SampleCallback cb) {
    callback_ = std::move(cb);
}

bool DataStream::read_sample(DataSample& out) {
    return ring_.pop(out);
}

std::uint64_t DataStream::samples_captured() const noexcept {
    return samples_captured_.load(std::memory_order_relaxed);
}

std::uint64_t DataStream::samples_dropped() const noexcept {
    return samples_dropped_.load(std::memory_order_relaxed);
}

bool DataStream::is_running() const noexcept {
    return running_.load(std::memory_order_relaxed);
}

void DataStream::capture_loop() {
    using namespace std::chrono;
    const auto interval = nanoseconds(1'000'000'000ULL / cfg_.sample_rate_hz);
    auto next_tick      = steady_clock::now();

    while (running_.load(std::memory_order_relaxed)) {
        auto sample = DataSample::make(sequence_++, cfg_.channel_count);

        samples_captured_.fetch_add(1, std::memory_order_relaxed);

        if (callback_)
            callback_(sample);

        if (!ring_.push(sample))
            samples_dropped_.fetch_add(1, std::memory_order_relaxed);

        next_tick += interval;
        std::this_thread::sleep_until(next_tick);
    }
}

} // namespace daq
