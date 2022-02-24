#pragma once

#include <concepts>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <type_traits>

#include "thread_pool/thread_safe_queue.h"

namespace dp {

    namespace detail {
        template <class T>
        std::decay_t<T> decay_copy(T &&v) {
            return std::forward<T>(v);
        }

        // bind F and parameter pack into a nullary one shot. Lambda captures by value.
        template <typename... Args, typename F>
        auto bind(F &&f, Args &&...args) {
            return [f = decay_copy(std::forward<F>(f)),
                    ... args = decay_copy(std::forward<Args>(args))]() mutable -> decltype(auto) {
                return std::invoke(std::move(f), std::move(args)...);
            };
        }

    }  // namespace detail

    template <typename FunctionType = std::function<void()>>
    requires std::invocable<FunctionType> &&
        std::is_same_v<void, std::invoke_result_t<FunctionType>>
    class thread_pool {
      public:
        thread_pool(const unsigned int &number_of_threads = std::thread::hardware_concurrency()) {
            for (std::size_t i = 0; i < number_of_threads; ++i) {
                threads_.emplace_back([&](const std::stop_token stop_tok) {
                    do {
                        // check if we have task
                        if (queue_.empty()) {
                            // no tasks, so we wait instead of spinning
                            std::unique_lock lock(condition_mutex_);
                            condition_.wait(lock, stop_tok, [this]() { return !queue_.empty(); });
                        }

                        // ensure we have a task before getting task
                        // since the dtor notifies via the condition variable as well
                        if (!queue_.empty()) {
                            // get the task
                            auto task = queue_.pop();
                            // invoke the task
                            std::invoke(std::move(task));
                            // decrement in-flight counter
                            --in_flight_;
                        }
                    } while (!stop_tok.stop_requested());
                });
            }
        }

        ~thread_pool() {
            // wait for tasks to complete first
            do {
                std::this_thread::yield();
            } while (in_flight_ > 0);

            // stop all threads
            for (auto &thread : threads_) {
                thread.request_stop();
            }
            condition_.notify_all();
        }

        /// thread pool is non-copyable
        thread_pool(const thread_pool &) = delete;
        thread_pool &operator=(const thread_pool &) = delete;

        /**
         * @brief Enqueue a task into the thread pool that returns a result.
         * @tparam Function An invokable type.
         * @tparam Args Argument parameter pack
         * @tparam ReturnType The return type of the Function
         * @param f The callable function
         * @param args The parameters that will be passed (copied) to the function.
         * @return A std::future<ReturnType> that can be used to retrieve the returned value.
         */
        template <typename Function, typename... Args,
                  typename ReturnType = std::invoke_result_t<Function &&, Args &&...>>
        requires std::invocable<Function, Args...>
        [[nodiscard]] std::future<ReturnType> enqueue(Function f, Args... args) {
            /*
             * use shared promise here so that we don't break the promise later (until C++23)
             *
             * with C++23 we can do the following:
             *
             * std::promise<ReturnType> promise;
             * auto future = promise.get_future();
             * auto task = [func = std::move(f), ...largs = std::move(args),
                              promise = std::move(promise)]() mutable {...};
             */
            auto shared_promise = std::make_shared<std::promise<ReturnType>>();
            auto task = [func = std::move(f), ... largs = std::move(args),
                         promise = shared_promise]() { promise->set_value(func(largs...)); };

            // get the future before enqueuing the task
            auto future = shared_promise->get_future();
            // enqueue the task
            enqueue_task(std::move(task));
            return future;
        }

        /**
         * @brief Enqueue a task to be executed in the thread pool that returns void.
         * @tparam Function An invokable type.
         * @tparam Args Argument parameter pack for Function
         * @param func The callable to be executed
         * @param args Arguments that will be passed to the function.
         */
        template <typename Function, typename... Args>
        requires std::invocable<Function, Args...> &&
            std::is_same_v<void, std::invoke_result_t<Function &&, Args &&...>>
        void enqueue_detach(Function &&func, Args &&...args) {
            enqueue_task(detail::bind(std::forward<Function>(func), std::forward<Args>(args)...));
        }

      private:
        template <typename Function>
        void enqueue_task(Function &&f) {
            ++in_flight_;
            {
                std::lock_guard lock(condition_mutex_);
                queue_.push(std::forward<Function>(f));
            }
            condition_.notify_all();
        }

        std::condition_variable_any condition_;
        std::mutex condition_mutex_;
        std::vector<std::jthread> threads_;
        dp::thread_safe_queue<FunctionType> queue_;
        std::atomic<int64_t> in_flight_{0};
    };

    /**
     * @example mandelbrot/source/main.cpp
     * Example showing how to use thread pool with tasks that return a value. Outputs a PPM image of
     * a mandelbrot.
     */
}  // namespace dp
