#include <gtest/gtest.h>
#include "DataProcessor.h"
#include "DataSample.h"

using namespace daq;

static DataSample make_single_channel(double value) {
    auto s = DataSample::make(0, 1);
    s.channels[0] = value;
    return s;
}

TEST(DataProcessorTest, PassthroughWithNoStages) {
    DataProcessor proc;
    auto s = make_single_channel(3.14);
    EXPECT_TRUE(proc.process(s));
    EXPECT_EQ(proc.stats().samples_processed, 1u);
    EXPECT_EQ(proc.stats().samples_rejected,  0u);
}

TEST(DataProcessorTest, RangeFilterAccepts) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_range_filter(0, -1.0, 1.0));
    auto s = make_single_channel(0.5);
    EXPECT_TRUE(proc.process(s));
}

TEST(DataProcessorTest, RangeFilterRejects) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_range_filter(0, -1.0, 1.0));
    auto s = make_single_channel(2.0);
    EXPECT_FALSE(proc.process(s));
    EXPECT_EQ(proc.stats().samples_rejected, 1u);
}

TEST(DataProcessorTest, GainStage) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_gain(0, 2.0));
    auto s = make_single_channel(3.0);
    EXPECT_TRUE(proc.process(s));
    EXPECT_DOUBLE_EQ(s.channels[0], 6.0);
}

TEST(DataProcessorTest, MovingAverageSmooths) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_moving_average(0, 3));

    std::vector<double> inputs  = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> outputs;
    for (double v : inputs) {
        auto s = make_single_channel(v);
        proc.process(s);
        outputs.push_back(s.channels[0]);
    }
    // Window fills progressively: 1, 1.5, 2, 3, 4
    EXPECT_NEAR(outputs[0], 1.0, 1e-9);
    EXPECT_NEAR(outputs[1], 1.5, 1e-9);
    EXPECT_NEAR(outputs[2], 2.0, 1e-9);
    EXPECT_NEAR(outputs[3], 3.0, 1e-9);
    EXPECT_NEAR(outputs[4], 4.0, 1e-9);
}

TEST(DataProcessorTest, StatsAccumulate) {
    DataProcessor proc;
    for (double v : {1.0, 2.0, 3.0, 4.0, 5.0}) {
        auto s = make_single_channel(v);
        proc.process(s);
    }
    const auto& st = proc.stats();
    EXPECT_EQ(st.samples_processed, 5u);
    EXPECT_NEAR(st.mean,      3.0, 1e-9);
    EXPECT_NEAR(st.min_value, 1.0, 1e-9);
    EXPECT_NEAR(st.max_value, 5.0, 1e-9);
}

TEST(DataProcessorTest, BatchProcessing) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_range_filter(0, 0.0, 3.0));

    std::vector<DataSample> in, out;
    for (double v : {1.0, 5.0, 2.0, -1.0, 3.0})
        in.push_back(make_single_channel(v));

    std::size_t kept = proc.process_batch(in, out);
    EXPECT_EQ(kept, 3u);
    EXPECT_EQ(out.size(), 3u);
}

TEST(DataProcessorTest, ResetStats) {
    DataProcessor proc;
    auto s = make_single_channel(1.0);
    proc.process(s);
    proc.reset_stats();
    EXPECT_EQ(proc.stats().samples_processed, 0u);
}

TEST(DataProcessorTest, ClearStages) {
    DataProcessor proc;
    proc.add_stage(DataProcessor::make_range_filter(0, 0.0, 1.0));
    proc.clear_stages();
    auto s = make_single_channel(999.0);
    EXPECT_TRUE(proc.process(s)); // no stages, always passes
}
