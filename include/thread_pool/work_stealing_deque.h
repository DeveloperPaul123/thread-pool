#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace dp {

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_destructive_interference_size;
#else
    // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
    inline constexpr std::size_t hardware_destructive_interference_size =
        2 * sizeof(std::max_align_t);
#endif

    /**
     * @brief Chase-Lev work stealing queue
     * @details Support single producer, multiple consumer. The producer owns the back, consumers
     * own the top. Consumers can also take from the top of the queue. The queue is "lock-free" in
     * that it does not directly use mutexes or locks.
     *
     * This is an implementation of the deque described in "Correct and Efficient Work-Stealing for
     * Weak Memory Models" and "Dynamic Circular Work-Stealing Deque" by Chase,Lev.
     *
     */
    template <typename T>
        requires std::is_destructible_v<T>
    class work_stealing_deque final {
        /**
         * @brief Simple circular array buffer that can regrow
         * TODO: Leverage std::pmr facilities to automatically allocate/reclaim memory?
         */
        class circular_buffer final {
          public:
            explicit circular_buffer(const std::int64_t size) : size_(size), mask_(size - 1) {
                // size must be a power of 2
                assert((size % 2) == 0);

                buffer_ = std::make_unique_for_overwrite<T[]>(size_);
                pointer_.store(buffer_.get(), release);
            }

            [[nodiscard]] std::int64_t capacity() const noexcept { return size_; }

            void store(const std::size_t index, T value, std::memory_order order = acquire) noexcept
                requires std::is_move_assignable_v<T>
            {
                auto buf = pointer_.load(order);
                buf[index & mask_] = value;
            }

            T load(const std::size_t index, std::memory_order order = acquire) noexcept {
                auto buf = pointer_.load(order);
                return buf[index & mask_];
            }

            /**
             * @brief Resize the internal buffer. Copies [start, end) to the new buffer.
             * @param start The start index
             * @param end The end index
             */
            circular_buffer* resize(const std::size_t start, const std::size_t end) {
                auto temp = new circular_buffer(size_ * 2);
                for (std::size_t i = start; i != end; ++i) {
                    temp->store(i, load(i));
                }
                return temp;
            }

          private:
            std::int64_t size_;
            std::int64_t mask_;
            std::atomic<T*> pointer_;
            std::unique_ptr<T[]> buffer_;
        };

        constexpr static std::size_t default_count = 1024;
        alignas(hardware_destructive_interference_size) std::atomic_int64_t top_;
        alignas(hardware_destructive_interference_size) std::atomic_int64_t bottom_;
        alignas(hardware_destructive_interference_size) std::atomic<circular_buffer*> buffer_;

        std::vector<std::unique_ptr<circular_buffer>> garbage_{32};

        static constexpr std::memory_order relaxed = std::memory_order_relaxed;
        static constexpr std::memory_order acquire = std::memory_order_acquire;
        static constexpr std::memory_order consume = std::memory_order_consume;
        static constexpr std::memory_order release = std::memory_order_release;
        static constexpr std::memory_order seq_cst = std::memory_order_seq_cst;

      public:
        explicit work_stealing_deque(const std::size_t& capacity = default_count)
            : top_(0), bottom_(0), buffer_(new circular_buffer(capacity)) {}

        // queue is non-copyable
        work_stealing_deque(work_stealing_deque&) = delete;
        work_stealing_deque& operator=(work_stealing_deque&) = delete;

        [[nodiscard]] std::size_t capacity() const { return buffer_.load(relaxed)->capacity(); }
        [[nodiscard]] std::size_t size() const {
            const auto bottom = bottom_.load(relaxed);
            const auto top = top_.load(relaxed);
            return static_cast<std::size_t>(bottom >= top ? bottom - top : 0);
        }

        [[nodiscard]] bool empty() const { return size() == 0; }
        template <typename... Args>
        void push_bottom(Args&&... args) {
            // construct first in case it throws
            T value(std::forward<Args>(args)...);
            push_bottom(std::move(value));
        }

        void push_bottom(T value) {
            auto bottom = bottom_.load(relaxed);
            auto top = top_.load(acquire);
            auto buffer = buffer_.load(relaxed);

            if (buffer->capacity() < (bottom - top) + 1) {
                garbage_.emplace_back(std::exchange(buffer, buffer->resize(top, bottom)));
                buffer_.store(buffer, release);
            }

            buffer->store(bottom, std::move(value));

            // this synchronizes with other acquire fences
            // memory operations about this line cannot be reordered
            std::atomic_thread_fence(release);

            bottom_.store(bottom + 1, relaxed);
        }

        std::optional<T> take_bottom() {
            auto bottom = bottom_.load(relaxed) - 1;
            auto buffer = buffer_.load(relaxed);

            // prevent stealing
            bottom_.store(bottom, relaxed);

            // this synchronizes with other release fences
            // memory ops below this line cannot be reordered
            std::atomic_thread_fence(acquire);

            auto top = top_.load(relaxed);
            if (top <= bottom) {
                // queue isn't empty
                if (top == bottom) {
                    // there is only 1 item left in the queue, we need the CAS to succeed
                    // since another thread may be trying to steal and could steal before we're able
                    // to take the bottom
                    if (!top_.compare_exchange_strong(top, top + 1, seq_cst, relaxed)) {
                        // failed race
                        bottom_.store(bottom + 1, relaxed);
                        return std::nullopt;
                    }
                    bottom_.store(bottom + 1, relaxed);
                }
                // there is more than one item in the queue, we can take the bottom
                return buffer->load(bottom);
            }
            // queue is empty, reset bottom
            bottom_.store(bottom + 1, relaxed);
            return std::nullopt;
        }

        std::optional<T> pop_top() {
            auto top = top_.load(acquire);
            // this synchronizes with other release fences
            // memory ops below this line cannot be reordered with ops above this line
            std::atomic_thread_fence(acquire);
            const auto bottom = bottom_.load(acquire);

            if (top < bottom) {
                // non-empty queue
                auto buffer = buffer_.load(acquire);
                auto temp = buffer->load(top, acquire);
                if (!top_.compare_exchange_strong(top, top + 1, seq_cst, relaxed)) {
                    // failed the race
                    return std::nullopt;
                }
                return temp;
            } else {
                // deque is empty
                return std::nullopt;
            }
        }
    };
}  // namespace dp
