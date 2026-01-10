#pragma once

#include "DataSample.h"
#include "RingBuffer.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace daq {

struct StreamConfig {
    std::uint32_t sample_rate_hz{1000};
    std::uint16_t channel_count{1};
    std::size_t   buffer_capacity{4096}; // must be power of two
    std::string   stream_id{"default"};
};

class DataStream {
public:
    using SampleCallback = std::function<void(const DataSample&)>;

    explicit DataStream(StreamConfig cfg);
    ~DataStream();

    DataStream(const DataStream&)            = delete;
    DataStream& operator=(const DataStream&) = delete;

    void start();
    void stop();

    void set_callback(SampleCallback cb);

    // Non-blocking: returns false if buffer is empty
    bool read_sample(DataSample& out);

    std::uint64_t samples_captured() const noexcept;
    std::uint64_t samples_dropped()  const noexcept;
    bool          is_running()        const noexcept;

    const StreamConfig& config() const noexcept { return cfg_; }

private:
    void capture_loop();

    StreamConfig              cfg_;
    RingBuffer<DataSample>    ring_;
    SampleCallback            callback_;
    std::thread               capture_thread_;
    std::atomic<bool>         running_{false};
    std::atomic<std::uint64_t> samples_captured_{0};
    std::atomic<std::uint64_t> samples_dropped_{0};
    std::uint32_t              sequence_{0};
};

} // namespace daq
