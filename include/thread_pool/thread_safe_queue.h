#pragma once

#include <deque>
#include <mutex>
#include <optional>

namespace dp {
    template <typename T>
    class thread_safe_queue {
      public:
        using value_type = T;
        using size_type = typename std::deque<T>::size_type;

        thread_safe_queue() = default;

        void push(T&& value) {
            std::lock_guard lock(mutex_);
            data_.push_back(std::forward<T>(value));
        }

        [[nodiscard]] bool empty() const {
            std::lock_guard lock(mutex_);
            return data_.empty();
        }

        [[nodiscard]] std::optional<T> pop() {
            std::lock_guard lock(mutex_);
            if (data_.empty()) return std::nullopt;

            auto front = data_.front();
            data_.pop_front();
            return front;
        }

      private:
        using mutex_type = std::mutex;
        std::deque<T> data_{};
        mutable mutex_type mutex_{};
    };
}  // namespace dp
