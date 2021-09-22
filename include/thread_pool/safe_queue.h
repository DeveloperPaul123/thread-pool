#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace dp {
  template <typename T> class safe_queue {
  public:
    safe_queue() {}
    void push(T&& value) {
      std::lock_guard<mutex_type> lock(mutex_);
      data_.push(value);
      condition_variable_.notify_one();
    }
    bool empty() {
      std::lock_guard<mutex_type> lock(mutex_);
      return data_.empty();
    }

    T& front() {
      std::unique_lock<mutex_type> lock(mutex_);
      while (data_.empty()) {
        condition_variable_.wait(lock);
      }
      return data_.front();
    }

    const T& front() const {
      std::unique_lock<mutex_type> lock(mutex_);
      while (data_.empty()) {
        condition_variable_.wait(lock);
      }
      return data_.front();
    }

    void pop() {
      std::lock_guard<mutex_type> lock(mutex_);
      data_.pop();
    }

  private:
    using mutex_type = std::mutex;
    std::queue<T> data_;
    mutex_type mutex_;
    std::condition_variable condition_variable_;
  };
}  // namespace dp
