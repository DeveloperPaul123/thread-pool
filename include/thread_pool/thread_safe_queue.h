#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace dp {
    template <typename T>
    class thread_safe_queue {
      public:
        using value_type = T;
        using size_type = std::queue<T>::size_type;

        thread_safe_queue() = default;
        void push(T&& value) {
            std::lock_guard lock(mutex_);
            data_.push(std::forward<T>(value));
            condition_variable_.notify_all();
        }
        bool empty() {
            std::lock_guard lock(mutex_);
            return data_.empty();
        }

        [[nodiscard]] size_type size() {
            std::lock_guard lock(mutex_);
            return data_.size();
        }

        [[nodiscard]] T& front() {
            std::unique_lock lock(mutex_);
            condition_variable_.wait(lock, [this] { return !data_.empty(); });
            return data_.front();
        }

        [[nodiscard]] T& back() {
            std::unique_lock lock(mutex_);
            condition_variable_.wait(lock, [this] { return !data_.empty(); });
            return data_.back();
        }

        void pop() {
            std::unique_lock lock(mutex_);
            condition_variable_.wait(lock, [this] { return !data_.empty(); });
            data_.pop();
        }

      private:
        using mutex_type = std::mutex;
        std::queue<T> data_;
        mutable mutex_type mutex_{};
        std::condition_variable condition_variable_{};
    };
}  // namespace dp
