#include <gtest/gtest.h>
#include "Instrument.h"

using namespace daq;

TEST(SimulatedInstrumentTest, OpenClose) {
    SimulatedInstrument inst(4);
    EXPECT_EQ(inst.state(), InstrumentState::Idle);
    EXPECT_TRUE(inst.open());
    EXPECT_TRUE(inst.is_open());
    inst.close();
    EXPECT_EQ(inst.state(), InstrumentState::Idle);
}

TEST(SimulatedInstrumentTest, ReadSampleRequiresOpen) {
    SimulatedInstrument inst(4);
    DataSample s;
    EXPECT_FALSE(inst.read_sample(s)); // not open
}

TEST(SimulatedInstrumentTest, ReadSamplePopulatesChannels) {
    SimulatedInstrument inst(4, 1.0, 0.0);
    inst.open();
    DataSample s;
    EXPECT_TRUE(inst.read_sample(s));
    EXPECT_EQ(s.channel_count, 4);
    EXPECT_EQ(inst.state(), InstrumentState::Acquiring);
}

TEST(SimulatedInstrumentTest, SequenceMonotonicallyIncreases) {
    SimulatedInstrument inst(2);
    inst.open();
    DataSample a, b;
    inst.read_sample(a);
    inst.read_sample(b);
    EXPECT_LT(a.sequence_id, b.sequence_id);
}

TEST(SimulatedInstrumentTest, AmplitudeBound) {
    // With zero noise the signal must stay within [-amplitude, +amplitude]
    SimulatedInstrument inst(1, 2.0, 0.0);
    inst.open();
    for (int i = 0; i < 200; ++i) {
        DataSample s;
        ASSERT_TRUE(inst.read_sample(s));
        EXPECT_LE(std::abs(s.channels[0]), 2.0 + 1e-9);
    }
}

TEST(SimulatedInstrumentTest, BadChannelCountThrows) {
    EXPECT_THROW(SimulatedInstrument(0),                      std::invalid_argument);
    EXPECT_THROW(SimulatedInstrument(MAX_CHANNELS + 1),       std::invalid_argument);
}

TEST(SimulatedInstrumentTest, InfoFields) {
    SimulatedInstrument inst(3);
    const auto info = inst.info();
    EXPECT_EQ(info.max_channels, 3);
    EXPECT_FALSE(info.serial_number.empty());
}
