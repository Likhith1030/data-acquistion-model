#pragma once

#include <chrono>
#include <cstdint>
#include <array>

namespace daq {

static constexpr std::size_t MAX_CHANNELS = 16;

struct DataSample {
    using Clock     = std::chrono::steady_clock;
    using Timestamp = std::chrono::time_point<Clock>;

    Timestamp                       timestamp{};
    std::array<double, MAX_CHANNELS> channels{};
    std::uint16_t                   channel_count{0};
    std::uint32_t                   sequence_id{0};
    bool                            valid{true};

    static DataSample make(std::uint32_t seq, std::uint16_t n_channels) {
        DataSample s;
        s.timestamp     = Clock::now();
        s.sequence_id   = seq;
        s.channel_count = n_channels;
        s.channels.fill(0.0);
        return s;
    }
};

} // namespace daq
