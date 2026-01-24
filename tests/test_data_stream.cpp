#include <gtest/gtest.h>
#include "DataStream.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace daq;

TEST(DataStreamTest, DefaultConstruction) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 100;
    cfg.channel_count   = 2;
    cfg.buffer_capacity = 256;
    EXPECT_NO_THROW(DataStream stream(cfg));
}

TEST(DataStreamTest, InvalidConfigThrows) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 0;
    cfg.channel_count   = 1;
    cfg.buffer_capacity = 64;
    EXPECT_THROW(DataStream s(cfg), std::invalid_argument);
}

TEST(DataStreamTest, StartStop) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 500;
    cfg.channel_count   = 1;
    cfg.buffer_capacity = 256;
    DataStream stream(cfg);
    stream.start();
    EXPECT_TRUE(stream.is_running());
    stream.stop();
    EXPECT_FALSE(stream.is_running());
}

TEST(DataStreamTest, SamplesAccumulate) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 1000;
    cfg.channel_count   = 1;
    cfg.buffer_capacity = 4096;
    DataStream stream(cfg);
    stream.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stream.stop();
    // Expect roughly 100 samples; allow generous ±50% margin for CI variance
    const auto captured = stream.samples_captured();
    EXPECT_GE(captured, 50u);
    EXPECT_LE(captured, 200u);
}

TEST(DataStreamTest, CallbackFired) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 200;
    cfg.channel_count   = 1;
    cfg.buffer_capacity = 256;
    DataStream stream(cfg);

    std::atomic<std::uint64_t> cb_count{0};
    stream.set_callback([&](const DataSample&) { cb_count.fetch_add(1); });
    stream.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stream.stop();
    EXPECT_GE(cb_count.load(), 10u);
}

TEST(DataStreamTest, ReadSampleNonBlocking) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 1000;
    cfg.channel_count   = 2;
    cfg.buffer_capacity = 256;
    DataStream stream(cfg);
    stream.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stream.stop();

    DataSample s;
    bool got = stream.read_sample(s);
    EXPECT_TRUE(got);
    EXPECT_EQ(s.channel_count, 2);
}

TEST(DataStreamTest, DoubleStartIsIdempotent) {
    StreamConfig cfg;
    cfg.sample_rate_hz  = 100;
    cfg.channel_count   = 1;
    cfg.buffer_capacity = 256;
    DataStream stream(cfg);
    stream.start();
    stream.start(); // should not spawn second thread
    EXPECT_TRUE(stream.is_running());
    stream.stop();
}
