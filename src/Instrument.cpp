#include "Instrument.h"

#include <cmath>
#include <random>
#include <stdexcept>

namespace daq {

SimulatedInstrument::SimulatedInstrument(std::uint16_t channels,
                                         double        amplitude,
                                         double        noise_std)
    : amplitude_(amplitude), noise_std_(noise_std) {
    if (channels == 0 || channels > MAX_CHANNELS)
        throw std::invalid_argument("channel count out of range");
    info_.name              = "SimulatedInstrument";
    info_.model             = "SIM-1000";
    info_.serial_number     = "SN-000001";
    info_.max_channels      = channels;
    info_.max_sample_rate_hz = 100'000.0;
}

bool SimulatedInstrument::open() {
    if (state_ != InstrumentState::Idle) return false;
    state_ = InstrumentState::Configured;
    return true;
}

void SimulatedInstrument::close() {
    state_ = InstrumentState::Idle;
}

bool SimulatedInstrument::read_sample(DataSample& out) {
    if (state_ == InstrumentState::Idle || state_ == InstrumentState::Error)
        return false;

    thread_local std::mt19937                     rng{std::random_device{}()};
    thread_local std::normal_distribution<double> noise{0.0, 1.0};

    out = DataSample::make(seq_++, info_.max_channels);

    constexpr double TWO_PI = 6.283185307179586;
    phase_ += TWO_PI * 10.0 / 1000.0; // 10 Hz tone at 1 kHz sample rate
    if (phase_ > TWO_PI) phase_ -= TWO_PI;

    for (std::uint16_t ch = 0; ch < info_.max_channels; ++ch) {
        const double phase_offset = ch * (TWO_PI / info_.max_channels);
        out.channels[ch] = amplitude_ * std::sin(phase_ + phase_offset)
                         + noise_std_ * noise(rng);
    }

    state_ = InstrumentState::Acquiring;
    return true;
}

} // namespace daq
