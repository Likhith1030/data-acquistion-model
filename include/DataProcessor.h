#pragma once

#include "DataSample.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace daq {

struct ProcessingStats {
    std::uint64_t samples_processed{0};
    std::uint64_t samples_rejected{0};
    double        mean{0.0};
    double        variance{0.0};
    double        min_value{0.0};
    double        max_value{0.0};
};

// Pipeline stage: receives a sample, optionally transforms it, returns false to drop it.
using ProcessingStage = std::function<bool(DataSample&)>;

class DataProcessor {
public:
    DataProcessor() = default;

    void add_stage(ProcessingStage stage);
    void clear_stages();

    // Returns true if the sample survived all stages.
    bool process(DataSample& sample);

    // Batch-process; fills `out` with surviving samples.
    std::size_t process_batch(std::vector<DataSample>& samples,
                              std::vector<DataSample>& out);

    const ProcessingStats& stats() const noexcept { return stats_; }
    void reset_stats();

    // Built-in stage factories
    static ProcessingStage make_range_filter(std::size_t channel, double lo, double hi);
    static ProcessingStage make_moving_average(std::size_t channel, std::size_t window);
    static ProcessingStage make_gain(std::size_t channel, double gain);

private:
    void update_stats(const DataSample& s);

    std::vector<ProcessingStage> stages_;
    ProcessingStats              stats_;

    // Welford online variance state
    double welf_mean_{0.0};
    double welf_m2_{0.0};
};

} // namespace daq
