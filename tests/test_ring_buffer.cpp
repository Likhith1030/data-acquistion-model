#include <gtest/gtest.h>
#include "RingBuffer.h"

using namespace daq;

TEST(RingBufferTest, ConstructorRejectsBadCapacity) {
    EXPECT_THROW(RingBuffer<int>(0),  std::invalid_argument);
    EXPECT_THROW(RingBuffer<int>(3),  std::invalid_argument);
    EXPECT_THROW(RingBuffer<int>(6),  std::invalid_argument);
    EXPECT_NO_THROW(RingBuffer<int>(8));
}

TEST(RingBufferTest, InitiallyEmpty) {
    RingBuffer<int> rb(8);
    EXPECT_TRUE(rb.empty());
    EXPECT_EQ(rb.size(), 0u);
}

TEST(RingBufferTest, PushPop) {
    RingBuffer<int> rb(8);
    EXPECT_TRUE(rb.push(42));
    EXPECT_EQ(rb.size(), 1u);

    int val = 0;
    EXPECT_TRUE(rb.pop(val));
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(rb.empty());
}

TEST(RingBufferTest, FillToCapacity) {
    RingBuffer<int> rb(4);
    EXPECT_TRUE(rb.push(1));
    EXPECT_TRUE(rb.push(2));
    EXPECT_TRUE(rb.push(3));
    // 4th slot is reserved as sentinel; buffer appears full
    EXPECT_FALSE(rb.push(4));
    EXPECT_TRUE(rb.full());
}

TEST(RingBufferTest, FIFOOrdering) {
    RingBuffer<int> rb(8);
    for (int i = 0; i < 5; ++i) rb.push(i);
    for (int i = 0; i < 5; ++i) {
        int v = -1;
        ASSERT_TRUE(rb.pop(v));
        EXPECT_EQ(v, i);
    }
    EXPECT_TRUE(rb.empty());
}

TEST(RingBufferTest, WrapAround) {
    RingBuffer<int> rb(4);
    rb.push(10); rb.push(20);
    int v = 0;
    rb.pop(v); EXPECT_EQ(v, 10);
    rb.push(30);
    rb.pop(v); EXPECT_EQ(v, 20);
    rb.pop(v); EXPECT_EQ(v, 30);
    EXPECT_TRUE(rb.empty());
}

TEST(RingBufferTest, PopFromEmpty) {
    RingBuffer<int> rb(8);
    int v = 99;
    EXPECT_FALSE(rb.pop(v));
    EXPECT_EQ(v, 99); // unchanged
}

TEST(RingBufferTest, CapacityReported) {
    RingBuffer<double> rb(16);
    EXPECT_EQ(rb.capacity(), 16u);
}
