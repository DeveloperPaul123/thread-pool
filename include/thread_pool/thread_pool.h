#pragma once

#include <atomic>
#include <concepts>
#include <coroutine>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <semaphore>
#include <thread>
#include <type_traits>

#include "thread_pool/thread_safe_queue.h"

namespace dp {
    namespace details {
        // leave clang detection out for now as there is not support for std::move_only_function
#if defined(__GNUC__) && !defined(__clang_major__)
#    define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#    define GCC_VERSION 0
#endif
#if (defined(_MSC_VER) && (_MSC_VER >= 1930) && (_MSVC_LANG > 202002L)) || (GCC_VERSION >= 120000)
#    define TP_HAS_MOVE_ONLY_FUNCTION_SUPPORT
        using default_function_type = std::move_only_function<void()>;
#else
        using default_function_type = std::function<void()>;
#endif
    }  // namespace details

    template <typename FunctionType = details::default_function_type>
    requires std::invocable<FunctionType> &&
        std::is_same_v<void, std::invoke_result_t<FunctionType>>
    class thread_pool {
      public:
        explicit thread_pool(
            const unsigned int &number_of_threads = std::thread::hardware_concurrency())
            : tasks_(number_of_threads) {
            for (std::size_t i = 0; i < number_of_threads; ++i) {
                try {
                    threads_.emplace_back([&, id = i](const std::stop_token &stop_tok) {
                        do {
                            // wait until signaled
                            tasks_[id].signal.acquire();

                            do {
                                // invoke the task
                                while (auto task = tasks_[id].tasks.pop()) {
                                    try {
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                    } catch (...) {
                                    }
                                }

                                // try to steal a task
                                for (std::size_t j = 1; j < tasks_.size(); ++j) {
                                    const std::size_t index = (id + j) % tasks_.size();
                                    if (auto task = tasks_[index].tasks.steal()) {
                                        // steal a task
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                        // stop stealing once we have invoked a stolen task
                                        break;
                                    }
                                }

                            } while (pending_tasks_.load(std::memory_order_acquire) > 0);
                        } while (!stop_tok.stop_requested());
                    });
                } catch (...) {
                    // catch all
                }
            }
        }

        ~thread_pool() {
            // stop all threads
            for (std::size_t i = 0; i < threads_.size(); ++i) {
                threads_[i].request_stop();
                tasks_[i].signal.release();
                threads_[i].join();
            }
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
#ifdef TP_HAS_MOVE_ONLY_FUNCTION_SUPPORT
            // we can do this in C++23 because we now have support for move only functions
            std::promise<ReturnType> promise;
            auto future = promise.get_future();
            auto task = [func = std::move(f), ... largs = std::move(args),
                         promise = std::move(promise)]() mutable {
                try {
                    promise.set_value(func(largs...));
                } catch (...) {
                    promise.set_exception(std::current_exception());
                }
            };
            enqueue_task(std::move(task));
            return future;
#else
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
                         promise = shared_promise]() {
                try {
                    promise->set_value(func(largs...));
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            };

            // get the future before enqueuing the task
            auto future = shared_promise->get_future();
            // enqueue the task
            enqueue_task(std::move(task));
            return future;
#endif
        }

        /**
         * @brief Enqueue a list of tasks into the thread pool.
         * @tparam Iterator An iterator type
         * @tparam IteratorType The underlying value type of the iterator
         * @param begin The start of the task range
         * @param end The end of the task range
         */
        template <typename Iterator,
                  typename IteratorType = typename std::iterator_traits<Iterator>::value_type>
        requires std::input_iterator<Iterator> && std::invocable<IteratorType> &&
            std::is_same_v<void, std::invoke_result_t<IteratorType>>
        void enqueue(Iterator &&begin, Iterator &&end) {
            // simple range check
            if (begin >= end) return;

            // enqueue all the tasks
            enqueue_tasks(std::forward<Iterator>(begin), std::forward<Iterator>(end));
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
            enqueue_task(
                std::move([f = std::forward<Function>(func),
                           ... largs = std::forward<Args>(args)]() mutable -> decltype(auto) {
                    // suppress exceptions
                    try {
                        std::invoke(f, largs...);
                    } catch (...) {
                    }
                }));
        }

        /**
         * @brief Allows you to schedule coroutines to run on the thread pool.
         */
        auto schedule() {
            /// @brief Simple awaitable type that we can return.
            struct scheduled_operation {
                dp::thread_pool<> *thread_pool_;
                static bool await_ready() { return false; }
                void await_suspend(std::coroutine_handle<> handle) {
                    if (thread_pool_) {
                        thread_pool_->enqueue_detach([](std::coroutine_handle<> h) { h.resume(); },
                                                     handle);
                    }
                }

                static void await_resume() {}
            };

            return scheduled_operation{this};
        }

      private:
        template <typename Function>
        [[maybe_unused]] auto assign_task_to_thread(Function &&f) -> std::size_t {
            tasks_[index_].tasks.push(std::forward<Function>(f));
            const auto &i = index_;
            if (++index_ >= tasks_.size()) index_ = 0;
            return i;
        }

        template <typename Function>
        void enqueue_task(Function &&f) {
            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
            const auto &assigned_index = assign_task_to_thread(std::forward<Function>(f));
            tasks_[assigned_index].signal.release();
        }

        template <typename Iterator,
                  typename IteratorType = typename std::iterator_traits<Iterator>::value_type>
        void enqueue_tasks(Iterator &&begin, Iterator &&end) {
            // get the count of tasks
            const auto &tasks = std::distance(begin, end);
            pending_tasks_.fetch_add(tasks, std::memory_order_relaxed);

            // get the number of threads once and re-use
            const auto &task_size = tasks_.size();

            // start index of where we're adding tasks
            std::optional<std::size_t> start = std::nullopt;
            // total count of threads we need to wake up.
            const auto &count = ::std::min<std::size_t>(tasks, task_size);

            for (auto it = begin; it < end; ++it) {
                // push the task
                const auto &assigned_index = assign_task_to_thread(std::move([f = *it]() {
                    // suppress exceptions
                    try {
                        std::invoke(f);
                    } catch (...) {
                    }
                }));
                if (!start) start = assigned_index;
            }

            // release all the needed signals to wake all threads
            for (std::size_t j = 0; j < count; j++) {
                const auto &index = (start.value() + j) % task_size;
                tasks_[index].signal.release();
            }
        }

        struct task_item {
            dp::thread_safe_queue<FunctionType> tasks{};
            std::binary_semaphore signal{0};
        };

        std::vector<std::jthread> threads_;
        std::deque<task_item> tasks_;
        std::uint_fast8_t index_{0};
        std::atomic_int_fast64_t pending_tasks_{};
    };

    /**
     * @example mandelbrot/source/main.cpp
     * Example showing how to use thread pool with tasks that return a value. Outputs a PPM image of
     * a mandelbrot.
     */
}  // namespace dp
