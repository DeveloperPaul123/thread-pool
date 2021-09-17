#pragma once

#include <concepts>
#include <future>
#include <queue>
#include <semaphore>
#include <thread>

namespace dp {

  namespace detail {
    // See: https://en.cppreference.com/w/cpp/thread/thread/thread
    template <class T> std::decay_t<T> decay_copy(T &&v) { return std::forward<T>(v); }

    // Bind F and args... into a nullary one-shot lambda. Lambda captures by value.
    template <typename... Args, typename F> auto bind(F &&f, Args &&...args) {
      return [f = decay_copy(std::forward<F>(f)),
              ... args = decay_copy(std::forward<Args>(args))]() mutable -> decltype(auto) {
        return std::invoke(std::move(f), std::move(args)...);
      };
    }
  }  // namespace detail

  class thread_pool {
  public:
    thread_pool(const unsigned int &number_of_threads = std::thread::hardware_concurrency())
        : queues_(number_of_threads) {
      // TODO
      for (std::size_t i = 0; i < number_of_threads; ++i) {
        threads_.emplace_back([&, id = i](std::stop_token stop_tok) {
          do {
            auto &pair = queues_[i];
            pair.semaphore.acquire();
            auto &task = pair.tasks.front();
            pair.tasks.pop();
            std::invoke(std::move(task));
          } while (!stop_tok.stop_requested());
        });
      }
    }

    ~thread_pool() {
      for (auto &thread : threads_) {
        thread.request_stop();
      }
      for (auto &[semaphore, _] : queues_) {
        semaphore.release();
      }
    }

    template <typename Function, typename... Args,
              typename ReturnType = std::invoke_result_t<Function &&, Args &&...>>
    requires std::invocable<Function, Args...>
    [[nodiscard]] std::future<ReturnType> enqueue(Function &&f, Args &&...args) {
      std::packaged_task<ReturnType()> task(std::bind(
          [func = std::forward<Function>(f)](auto &&...args) -> ReturnType {
            return std::move(func)(decltype(args)(args)...);
          },
          std::forward<Args>(args)...));
      auto future = task.get_future();

      enqueue_task([t = std::move(task)]() { t(); });
      return future;
    }

    template <typename Function, typename... Args>
    requires std::invocable<Function, Args...>
    void enqueue_detach(Function &&func, Args &&...args) {
      enqueue_task(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    }

  private:
    using semaphore_type = std::counting_semaphore<4>;
    struct task_pair {
      semaphore_type semaphore{0};
      std::queue<std::function<void()>> tasks;
    };

    template <typename Function>

    void enqueue_task(Function &&f) {
      const std::size_t i = count_++ % queues_.size();

      queues_[i].tasks.push(std::forward<Function>(f));
      queues_[i].semaphore.release();
    }

    std::vector<std::jthread> threads_;
    std::vector<task_pair> queues_;
    std::size_t count_ = 0;
  };
}  // namespace dp
