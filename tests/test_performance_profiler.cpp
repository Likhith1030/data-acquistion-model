#include <gtest/gtest.h>
#include "PerformanceProfiler.h"

#include <thread>
#include <chrono>

using namespace daq;

TEST(PerformanceProfilerTest, RecordAndStats) {
    PerformanceProfiler prof;
    using ns = std::chrono::nanoseconds;
    prof.record("op", ns(1000));
    prof.record("op", ns(2000));
    prof.record("op", ns(3000));

    const auto s = prof.stats("op");
    EXPECT_EQ(s.sample_count, 3u);
    EXPECT_NEAR(s.mean_us, 2.0, 1e-6);
    EXPECT_NEAR(s.min_us,  1.0, 1e-6);
    EXPECT_NEAR(s.max_us,  3.0, 1e-6);
}

TEST(PerformanceProfilerTest, EmptyLabelReturnsZeroStats) {
    PerformanceProfiler prof;
    const auto s = prof.stats("nonexistent");
    EXPECT_EQ(s.sample_count, 0u);
    EXPECT_DOUBLE_EQ(s.mean_us, 0.0);
}

TEST(PerformanceProfilerTest, ResetSingleLabel) {
    PerformanceProfiler prof;
    prof.record("a", std::chrono::nanoseconds(5000));
    prof.record("b", std::chrono::nanoseconds(1000));
    prof.reset("a");
    EXPECT_EQ(prof.stats("a").sample_count, 0u);
    EXPECT_EQ(prof.stats("b").sample_count, 1u);
}

TEST(PerformanceProfilerTest, ResetAll) {
    PerformanceProfiler prof;
    prof.record("x", std::chrono::nanoseconds(1000));
    prof.record("y", std::chrono::nanoseconds(2000));
    prof.reset_all();
    EXPECT_EQ(prof.labels().size(), 0u);
}

TEST(PerformanceProfilerTest, ScopedTimerRecords) {
    PerformanceProfiler prof;
    {
        ScopedTimer t(prof, "sleep_op");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const auto s = prof.stats("sleep_op");
    EXPECT_EQ(s.sample_count, 1u);
    EXPECT_GE(s.mean_us, 500.0); // at least 0.5 ms
}

TEST(PerformanceProfilerTest, PercentileOrdering) {
    PerformanceProfiler prof;
    for (int i = 1; i <= 100; ++i)
        prof.record("pct", std::chrono::nanoseconds(i * 1000));

    const auto s = prof.stats("pct");
    EXPECT_LE(s.p50_us, s.p95_us);
    EXPECT_LE(s.p95_us, s.p99_us);
    EXPECT_LE(s.p99_us, s.max_us);
}
