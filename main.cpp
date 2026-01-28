#include "DataStream.h"
#include "DataProcessor.h"
#include "Instrument.h"
#include "PerformanceProfiler.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    using namespace daq;
    using namespace std::chrono_literals;

    std::cout << "=== Scientific Instrument Data Acquisition Demo ===\n\n";

    // --- Instrument ---
    SimulatedInstrument inst(4, 1.0, 0.02);
    if (!inst.open()) {
        std::cerr << "Failed to open instrument\n";
        return 1;
    }
    const auto info = inst.info();
    std::cout << "Instrument : " << info.name << " [" << info.model
              << " / " << info.serial_number << "]\n"
              << "Channels   : " << info.max_channels << "\n\n";

    // --- Processing pipeline ---
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_range_filter(0, -2.0, 2.0));
    proc.add_stage(DataProcessor::make_moving_average(0, 5));
    proc.add_stage(DataProcessor::make_gain(0, 10.0));

    // --- Profiler ---
    PerformanceProfiler profiler;

    // --- Stream ---
    StreamConfig cfg;
    cfg.stream_id       = "primary";
    cfg.sample_rate_hz  = 500;
    cfg.channel_count   = 4;
    cfg.buffer_capacity = 4096;

    DataStream stream(cfg);

    std::vector<DataSample> processed_samples;
    processed_samples.reserve(1024);

    stream.set_callback([&](const DataSample& raw) {
        ScopedTimer t(profiler, "process_sample");
        DataSample s = raw;
        if (proc.process(s))
            processed_samples.push_back(s);
    });

    std::cout << "Acquiring for 500 ms at " << cfg.sample_rate_hz << " Hz...\n";
    stream.start();
    std::this_thread::sleep_for(500ms);
    stream.stop();
    inst.close();

    // --- Report ---
    std::cout << "\n--- Acquisition Summary ---\n"
              << "Captured : " << stream.samples_captured() << " samples\n"
              << "Dropped  : " << stream.samples_dropped()  << " samples\n"
              << "Processed: " << proc.stats().samples_processed << " samples\n"
              << "Rejected : " << proc.stats().samples_rejected  << " samples\n";

    const auto& ps = proc.stats();
    std::cout << "\nCh-0 stats (after pipeline):\n"
              << "  Mean    : " << ps.mean     << "\n"
              << "  Variance: " << ps.variance << "\n"
              << "  Min     : " << ps.min_value << "\n"
              << "  Max     : " << ps.max_value << "\n";

    std::cout << "\n--- Latency Report ---\n";
    profiler.print_report();

    return 0;
}
