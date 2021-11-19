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

        // Bind F and args... into a nullary one-shot lambda. Lambda captures by value.
        template <typename... Args, typename F>
        auto bind(F &&f, Args &&...args) {
            return [f = decay_copy(std::forward<F>(f)),
                    ... args = decay_copy(std::forward<Args>(args))]() mutable -> decltype(auto) {
                return std::invoke(std::move(f), std::move(args)...);
            };
        }

        template <class Queue, class U = typename Queue::value_type>
        concept is_valid_queue = requires(Queue q) {
            { q.empty() } -> std::convertible_to<bool>;
            { q.front() } -> std::convertible_to<U &>;
            { q.back() } -> std::convertible_to<U &>;
            q.pop();
        };

        static_assert(detail::is_valid_queue<std::queue<int>>);
        static_assert(detail::is_valid_queue<dp::thread_safe_queue<int>>);
    }  // namespace detail

    template <template <class T> class Queue, typename FunctionType = std::function<void()>>
    requires std::invocable<FunctionType> &&
        std::is_same_v<void, std::invoke_result_t<FunctionType>> &&
        detail::is_valid_queue<Queue<FunctionType>>
    class thread_pool_impl {
      public:
        thread_pool_impl(
            const unsigned int &number_of_threads = std::thread::hardware_concurrency()) {
            for (std::size_t i = 0; i < number_of_threads; ++i) {
                queues_.push_back(std::make_unique<task_pair>());
                threads_.emplace_back([&, id = i](std::stop_token stop_tok) {
                    do {
                        // check if we have task
                        if (queues_[id]->tasks.empty()) {
                            // no tasks, so we wait instead of spinning
                            queues_[id]->semaphore.acquire();
                        }

                        // ensure we have a task before getting task
                        // since the dtor releases the semaphore as well
                        if (!queues_[id]->tasks.empty()) {
                            // get the task
                            auto &task = queues_[id]->tasks.front();
                            // invoke the task
                            std::invoke(std::move(task));
                            // remove task from the queue
                            queues_[id]->tasks.pop();
                        }
                    } while (!stop_tok.stop_requested());
                });
            }
        }

        ~thread_pool_impl() {
            // stop all threads
            for (std::size_t i = 0; i < threads_.size(); ++i) {
                threads_[i].request_stop();
                queues_[i]->semaphore.release();
                threads_[i].join();
            }
        }

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
        [[nodiscard]] std::future<ReturnType> enqueue(Function &&f, Args &&...args) {
            // use shared promise here so that we don't break the promise later
            auto shared_promise = std::make_shared<std::promise<ReturnType>>();
            // here we move arguments into a tuple since we cannot do an init-capture followed by an
            // ellipsis doing arguments = std::move(args)... would be better here, but currently not
            // possible. see: http://eel.is/c++draft/expr.prim.lambda#capture-17.sentence-2 see:
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0780r1.html
            auto task = [func = std::move(f), tup = std::make_tuple(std::move(args)...),
                         promise = shared_promise]() { promise->set_value(std::apply(func, tup)); };
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
        using semaphore_type = std::binary_semaphore;
        using task_type = FunctionType;
        struct task_pair {
            semaphore_type semaphore{0};
            Queue<task_type> tasks{};
        };

        template <typename Function>
        void enqueue_task(Function &&f) {
            const std::size_t i = count_++ % queues_.size();
            queues_[i]->tasks.push(std::forward<Function>(f));
            queues_[i]->semaphore.release();
        }

        std::vector<std::jthread> threads_;
        // have to use unique_ptr here because std::binary_semaphore is not move/copy
        // assignable/constructible
        std::vector<std::unique_ptr<task_pair>> queues_;
        std::size_t count_ = 0;
    };

    /**
     * @brief Thread pool class capable of queuing detached tasks and value returning tasks.
     * @details This is a default alias for the dp::thread_pool_impl
     */
    using thread_pool = thread_pool_impl<dp::thread_safe_queue>;

    /**
     * @example mandelbrot/source/main.cpp
     * Example showing how to use thread pool with tasks that return a value. Outputs a PPM image of
     * a mandelbrot.
     */
}  // namespace dp
