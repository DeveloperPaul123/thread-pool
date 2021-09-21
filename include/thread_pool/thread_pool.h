#pragma once

#include <concepts>
#include <future>
#include <memory>
#include <queue>
#include <semaphore>
#include <thread>
#include <type_traits>

namespace dp {

  namespace detail {
    template <class T> std::decay_t<T> decay_copy(T &&v) { return std::forward<T>(v); }

    // Bind F and args... into a nullary one-shot lambda. Lambda captures by value.
    template <typename... Args, typename F> auto bind(F &&f, Args &&...args) {
      return [f = decay_copy(std::forward<F>(f)),
              ... args = decay_copy(std::forward<Args>(args))]() mutable -> decltype(auto) {
        return std::invoke(std::move(f), std::move(args)...);
      };
    }

    template <typename Function>
    requires std::invocable<Function>
    class task_wrapper {
    public:
      task_wrapper(Function &&function) : function_(std::move(function)) {}

      void operator()() && {
        try {
          if constexpr (std::is_same_v<void, std::invoke_result_t<Function>>) {
            std::invoke(std::move(function_));
            result_promise_.set_value();
          } else {
            result_promise_.set_value(std::invoke(std::move(function_)));
          }
        } catch (...) {
          result_promise_.set_exception(std::current_exception());
        }
      }

      [[nodiscard]] std::future<std::invoke_result_t<Function>> get_future() {
        return result_promise_.get_future();
      }

    private:
      Function function_;
      std::promise<std::invoke_result_t<Function>> result_promise_;
    };
  }  // namespace detail

  class thread_pool {
  public:
    thread_pool(const unsigned int &number_of_threads = std::thread::hardware_concurrency()) {
      for (std::size_t i = 0; i < number_of_threads; ++i) {
        queues_.push_back(std::make_unique<task_pair>());
        threads_.emplace_back([&, id = i](std::stop_token stop_tok) {
          do {
            // TODO: Check if this is correct
            if (queues_.empty()) {
              break;
            }
            queues_[id]->semaphore.acquire();
            auto &task = queues_[id]->tasks.front();
            queues_[id]->tasks.pop();
            std::invoke(std::move(task));
          } while (!stop_tok.stop_requested());
        });
      }
    }

    ~thread_pool() {
      for (auto &thread : threads_) {
        thread.request_stop();
      }
      for (auto &pair : queues_) {
        pair->semaphore.release();
      }
    }

    template <typename Function, typename... Args,
              typename ReturnType = std::invoke_result_t<Function &&, Args &&...>>
    requires std::invocable<Function, Args...>
    [[nodiscard]] std::future<ReturnType> enqueue(Function &&f, Args &&...args) {
      auto shared_promise = std::make_shared<std::promise<ReturnType>>();

      auto task = [&, shared_promise]() {
        std::invoke(std::forward<Function>(f), std::forward<Args>(args)...);
      };

      auto future = shared_promise->get_future();

      enqueue_task(std::move(task));
      return future;
    }

    template <typename Function, typename... Args>
    requires std::invocable<Function, Args...>
    void enqueue_detach(Function &&func, Args &&...args) {
      enqueue_task(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    }

  private:
    using semaphore_type = std::binary_semaphore;
    struct task_pair {
      semaphore_type semaphore{0};
      std::queue<std::function<void()>> tasks{};
    };

    template <typename Function>

    void enqueue_task(Function &&f) {
      const std::size_t i = count_++ % queues_.size();

      queues_[i]->tasks.push(std::forward<Function>(f));
      queues_[i]->semaphore.release();
    }

    std::vector<std::jthread> threads_;
    std::vector<std::unique_ptr<task_pair>> queues_;
    std::size_t count_ = 0;
  };
}  // namespace dp
