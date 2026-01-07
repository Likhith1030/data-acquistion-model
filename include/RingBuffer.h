#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace daq {

// Lock-free single-producer single-consumer ring buffer.
// Capacity must be a power of two.
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : capacity_(capacity), mask_(capacity - 1), buffer_(capacity),
          head_(0), tail_(0) {
        if (capacity == 0 || (capacity & mask_) != 0)
            throw std::invalid_argument("RingBuffer capacity must be a power of two");
    }

    bool push(const T& item) noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) & mask_ ? (head + 1) : capacity_;
        if ((head + 1) == tail_.load(std::memory_order_acquire) + capacity_)
            return false; // full
        buffer_[head & mask_] = item;
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) noexcept {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire))
            return false; // empty
        item = buffer_[tail & mask_];
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    std::size_t size() const noexcept {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);
        return head - tail;
    }

    bool empty() const noexcept { return size() == 0; }
    bool full()  const noexcept { return size() >= capacity_; }
    std::size_t capacity() const noexcept { return capacity_; }

private:
    const std::size_t capacity_;
    const std::size_t mask_;
    std::vector<T> buffer_;
    alignas(64) std::atomic<std::size_t> head_;
    alignas(64) std::atomic<std::size_t> tail_;
};

} // namespace daq
