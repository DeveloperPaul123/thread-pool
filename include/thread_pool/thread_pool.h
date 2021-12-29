#pragma once

#include <concepts>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <semaphore>
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
        thread_pool(const unsigned int &number_of_threads = std::thread::hardware_concurrency())
            : queues_(number_of_threads) {
            for (std::size_t i = 0; i < number_of_threads; ++i) {
                threads_.emplace_back([&, id = i](std::stop_token stop_tok) {
                    do {
                        // check if we have task
                        if (queues_[id].tasks.empty()) {
                            // no tasks, so we wait instead of spinning
                            queues_[id].semaphore.acquire();
                        }

                        // ensure we have a task before getting task
                        // since the dtor releases the semaphore as well
                        if (!queues_[id].tasks.empty()) {
                            // get the task
                            auto &task = queues_[id].tasks.front();
                            // invoke the task
                            std::invoke(std::move(task));
                            // decrement in-flight counter
                            --in_flight_;
                            // remove task from the queue
                            queues_[id].tasks.pop();
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
            for (std::size_t i = 0; i < threads_.size(); ++i) {
                threads_[i].request_stop();
                queues_[i].semaphore.release();
                threads_[i].join();
            }
        }

        /// thread pool is non-copyable
        thread_pool(const thread_pool &) = delete;
        thread_pool &operator=(const thread_pool &) = delete;

        /**
         * @brief Enqueue a task into the thread pool that returns a result.
         * @tparam Function An invocable type.
         * @tparam ...Args Argument parameter pack
         * @tparam ReturnType The return type of the Function
         * @param f The callable function
         * @param ...args The parameters that will be passed (copied) to the function.
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
             * auto task = [func = std::move(f), ... largs = std::move(args),
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
         * @tparam Function An invocable type.
         * @tparam ...Args Argument parameter pack for Function
         * @param func The callable to be executed
         * @param ...args Arguments that wiill be passed to the function.
         */
        template <typename Function, typename... Args>
        requires std::invocable<Function, Args...> &&
            std::is_same_v<void, std::invoke_result_t<Function &&, Args &&...>>
        void enqueue_detach(Function &&func, Args &&...args) {
            enqueue_task(detail::bind(std::forward<Function>(func), std::forward<Args>(args)...));
        }

      private:
        struct task_queue {
            std::binary_semaphore semaphore{0};
            dp::thread_safe_queue<FunctionType> tasks{};
        };

        template <typename Function>
        void enqueue_task(Function &&f) {
            const std::size_t i = count_++ % queues_.size();
            ++in_flight_;
            queues_[i].tasks.push(std::forward<Function>(f));
            queues_[i].semaphore.release();
        }

        std::vector<std::jthread> threads_;
        std::deque<task_queue> queues_;
        std::size_t count_ = 0;
        std::atomic<int64_t> in_flight_{0};
    };

    /**
     * @example mandelbrot/source/main.cpp
     * Example showing how to use thread pool with tasks that return a value. Outputs a PPM image of
     * a mandelbrot.
     */
}  // namespace dp
