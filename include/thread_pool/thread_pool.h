#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <semaphore>
#include <thread>
#include <tuple>  // std::ignore
#include <type_traits>

#ifdef __has_include
#    if __has_include(<version>)
#        include <version>
#    endif
#endif

#include "thread_safe_queue.h"

namespace dp {
    namespace details {

#ifdef __cpp_lib_move_only_function
        using default_function_type = std::move_only_function<void()>;
#else
        using default_function_type = std::function<void()>;
#endif
    }  // namespace details

    template <typename FunctionType = details::default_function_type,
              typename ThreadType = std::jthread>
        requires std::invocable<FunctionType> &&
                 std::is_same_v<void, std::invoke_result_t<FunctionType>>
    class thread_pool {
      public:
        template <typename InitializationFunction = std::function<void(std::size_t)>>
            requires std::invocable<InitializationFunction, std::size_t> &&
                     std::is_same_v<void, std::invoke_result_t<InitializationFunction, std::size_t>>
        explicit thread_pool(
            const unsigned int& number_of_threads = std::thread::hardware_concurrency(),
            InitializationFunction init = [](std::size_t) {})
            : tasks_(number_of_threads) {
            std::size_t current_id = 0;
            for (std::size_t i = 0; i < number_of_threads; ++i) {
                priority_queue_.push_back(size_t(current_id));
                try {
                    threads_.emplace_back([&, id = current_id,
                                           init](const std::stop_token& stop_tok) {
                        // invoke the init function on the thread
                        try {
                            std::invoke(init, id);
                        } catch (...) {
                            // suppress exceptions
                        }

                        do {
                            // wait until signaled
                            tasks_[id].signal.acquire();

                            do {
                                // invoke the task
                                while (auto task = tasks_[id].tasks.pop_front()) {
                                    // decrement the unassigned tasks as the task is now going
                                    // to be executed
                                    unassigned_tasks_.fetch_sub(1, std::memory_order_release);
                                    // invoke the task
                                    std::invoke(std::move(task.value()));
                                    // the above task can push more work onto the pool, so we
                                    // only decrement the in flights once the task has been
                                    // executed because now it's now longer "in flight". Wake
                                    // wait_for_tasks() only on the transition to zero: checking
                                    // in_flight_tasks_ separately from this decrement (as a prior
                                    // version did) races with enqueue_task()'s own read-then-set
                                    // of the same counter.
                                    if (in_flight_tasks_.fetch_sub(1, std::memory_order_acq_rel) ==
                                        1) {
                                        in_flight_tasks_.notify_all();
                                    }
                                }

                                // try to steal a task
                                for (std::size_t j = 1; j < tasks_.size(); ++j) {
                                    const std::size_t index = (id + j) % tasks_.size();
                                    if (auto task = tasks_[index].tasks.steal()) {
                                        // steal a task
                                        unassigned_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                        if (in_flight_tasks_.fetch_sub(
                                                1, std::memory_order_acq_rel) == 1) {
                                            in_flight_tasks_.notify_all();
                                        }
                                        // stop stealing once we have invoked a stolen task
                                        break;
                                    }
                                }
                                // check if there are any unassigned tasks before rotating to the
                                // front and waiting for more work
                            } while (unassigned_tasks_.load(std::memory_order_acquire) > 0);

                            priority_queue_.rotate_to_front(id);

                        } while (!stop_tok.stop_requested());
                    });
                    // increment the thread id
                    ++current_id;

                } catch (...) {
                    // catch all

                    // remove one item from the tasks
                    tasks_.pop_back();

                    // remove our thread from the priority queue
                    std::ignore = priority_queue_.pop_back();
                }
            }
        }

        ~thread_pool() {
            wait_for_tasks();

            // stop all threads
            for (std::size_t i = 0; i < threads_.size(); ++i) {
                threads_[i].request_stop();
                tasks_[i].signal.release();
                threads_[i].join();
            }
        }

        /// thread pool is non-copyable
        thread_pool(const thread_pool&) = delete;
        thread_pool& operator=(const thread_pool&) = delete;

        /**
         * @brief Enqueue a task into the thread pool that returns a result.
         * @details Note that task execution begins once the task is enqueued.
         * @tparam Function An invokable type.
         * @tparam Args Argument parameter pack
         * @tparam ReturnType The return type of the Function
         * @param f The callable function
         * @param args The parameters that will be passed (copied) to the function.
         * @return A std::future<ReturnType> that can be used to retrieve the returned value.
         */
        template <typename Function, typename... Args,
                  typename ReturnType = std::invoke_result_t<Function&&, Args&&...>>
            requires std::invocable<Function, Args...>
        [[nodiscard]] std::future<ReturnType> enqueue(Function f, Args... args) {
#ifdef __cpp_lib_move_only_function
            // we can do this in C++23 because we now have support for move only functions
            std::promise<ReturnType> promise;
            auto future = promise.get_future();
            auto task = [func = std::move(f), ... largs = std::move(args),
                         promise = std::move(promise)]() mutable {
                try {
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        func(largs...);
                        promise.set_value();
                    } else {
                        promise.set_value(func(largs...));
                    }
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
                    if constexpr (std::is_same_v<ReturnType, void>) {
                        func(largs...);
                        promise->set_value();
                    } else {
                        promise->set_value(func(largs...));
                    }

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
         * @brief Enqueue a task to be executed in the thread pool. Any return value of the function
         * will be ignored.
         * @tparam Function An invokable type.
         * @tparam Args Argument parameter pack for Function
         * @param func The callable to be executed
         * @param args Arguments that will be passed to the function.
         */
        template <typename Function, typename... Args>
            requires std::invocable<Function, Args...>
        void enqueue_detach(Function&& func, Args&&... args) {
            enqueue_task(
                std::move([f = std::forward<Function>(func),
                           ... largs = std::forward<Args>(args)]() mutable -> decltype(auto) {
                    // suppress exceptions
                    try {
                        if constexpr (std::is_same_v<void,
                                                     std::invoke_result_t<Function&&, Args&&...>>) {
                            std::invoke(f, largs...);
                        } else {
                            // the function returns an argument, but can be ignored
                            std::ignore = std::invoke(f, largs...);
                        }
                    } catch (...) {
                    }
                }));
        }

        /**
         * @brief Returns the number of threads in the pool.
         *
         * @return std::size_t The number of threads in the pool.
         */
        [[nodiscard]] auto size() const { return threads_.size(); }

        /**
         * @brief Wait for all tasks to finish.
         * @details This function will block until all tasks have been completed.
         */
        void wait_for_tasks() {
            auto current = in_flight_tasks_.load(std::memory_order_acquire);
            while (current > 0) {
                // wait() blocks only while the value still equals `current`; a worker's
                // fetch_sub() notifies exactly when the count transitions to zero (see the
                // worker loop above), so there is no separate flag to fall out of sync with
                // in_flight_tasks_ itself.
                in_flight_tasks_.wait(current, std::memory_order_acquire);
                current = in_flight_tasks_.load(std::memory_order_acquire);
            }
        }

        /**
         * @brief Makes best-case attempt to clear all tasks from the thread_pool
         * @details Note that this does not guarantee that all tasks will be cleared, as currently
         * running tasks could add additional tasks. Also a thread could steal a task from another
         * in the middle of this.
         * @return number of tasks cleared
         */
        size_t clear_tasks() {
            size_t removed_task_count{0};
            for (auto& task_list : tasks_) {
                removed_task_count += task_list.tasks.clear();
            }
            in_flight_tasks_.fetch_sub(removed_task_count, std::memory_order_release);
            unassigned_tasks_.fetch_sub(removed_task_count, std::memory_order_release);

            return removed_task_count;
        }

      private:
        template <typename Function>
        void enqueue_task(Function&& f) {
            auto i_opt = priority_queue_.copy_front_and_rotate_to_back();
            if (!i_opt.has_value()) {
                // would only be a problem if there are zero threads
                return;
            }
            // get the index
            auto i = *(i_opt);

            // increment the unassigned tasks and in flight tasks
            unassigned_tasks_.fetch_add(1, std::memory_order_release);
            in_flight_tasks_.fetch_add(1, std::memory_order_release);

            // assign work
            tasks_[i].tasks.push_back(std::forward<Function>(f));
            tasks_[i].signal.release();
        }

        struct task_item {
            dp::thread_safe_queue<FunctionType> tasks{};
            // A worker acquires this once per wake-up and then drains its entire queue, so
            // enqueue_task() can release it many times before a matching acquire. It must
            // therefore be a counting semaphore: releasing a binary_semaphore past its max()
            // of 1 is a precondition violation (undefined behavior), and aborts under
            // libstdc++ assertions, which are enabled automatically in unoptimized builds.
            std::counting_semaphore<> signal{0};
            static_assert(decltype(signal)::max() > 1,
                          "task_item::signal must be a counting semaphore; enqueue_task() "
                          "releases it once per task without a matching acquire.");
        };

        std::vector<ThreadType> threads_;
        std::deque<task_item> tasks_;
        dp::thread_safe_queue<std::size_t> priority_queue_;
        // guarantee these get zero-initialized
        std::atomic_int_fast64_t unassigned_tasks_{0}, in_flight_tasks_{0};
    };

    /**
     * @example mandelbrot/source/main.cpp
     * Example showing how to use thread pool with tasks that return a value. Outputs a PPM image of
     * a mandelbrot.
     */
}  // namespace dp
