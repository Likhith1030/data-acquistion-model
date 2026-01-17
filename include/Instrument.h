#pragma once

#include "DataSample.h"

#include <cstdint>
#include <string>

namespace daq {

enum class InstrumentState { Idle, Configured, Acquiring, Error };

struct InstrumentInfo {
    std::string   name;
    std::string   model;
    std::string   serial_number;
    std::uint16_t max_channels{1};
    double        max_sample_rate_hz{0.0};
};

// Abstract base for pluggable instrument backends
class Instrument {
public:
    virtual ~Instrument() = default;

    virtual bool open()  = 0;
    virtual void close() = 0;

    // Synchronously read one sample; returns false on error.
    virtual bool read_sample(DataSample& out) = 0;

    virtual InstrumentState state() const noexcept = 0;
    virtual InstrumentInfo  info()  const noexcept = 0;

    bool is_open() const noexcept {
        return state() == InstrumentState::Configured ||
               state() == InstrumentState::Acquiring;
    }
};

// Simulated instrument for testing and demonstration
class SimulatedInstrument : public Instrument {
public:
    explicit SimulatedInstrument(std::uint16_t channels = 4,
                                 double        amplitude = 1.0,
                                 double        noise_std = 0.01);

    bool open()  override;
    void close() override;
    bool read_sample(DataSample& out) override;

    InstrumentState state() const noexcept override { return state_; }
    InstrumentInfo  info()  const noexcept override { return info_; }

private:
    InstrumentInfo  info_;
    InstrumentState state_{InstrumentState::Idle};
    double          amplitude_;
    double          noise_std_;
    std::uint32_t   seq_{0};
    double          phase_{0.0};
};

} // namespace daq
