#pragma once

#include <concepts>
#include <functional>
#include <stop_token>
#include <thread>
#include <type_traits>

namespace dp {
    namespace details {
        template <typename T, typename Function, typename... Args>
        concept JoinableThread = requires(T t) { t.join(); };

        static_assert(JoinableThread<std::jthread, std::function<void()>>);
    }  // namespace details

    struct native_thread_attributes {
        std::size_t stack_size_kb{1024};
    };

#if defined(__APPLE__) || defined(TP_USE_PTHREADS)

#    include <pthread.h>

    class native_thread {
      public:
        template <typename Function, typename... Args>
        native_thread(Function &&func, Args &&...args,
                      const native_thread_attributes params = native_thread_attributes{})
            : stop_source_(), thread_(0) {
            pthread_attr_t attr_storage, *attr = &attr_storage;
            pthread_attr_init(attr);
            pthread_attr_setstacksize(attr, params.stack_size_kb * 1024);

            using null_op_t = std::function<void()>;
            null_op_t *func_ptr;

            if constexpr (std::is_invocable_v<std::decay_t<Function>, std::stop_token,
                                              std::decay_t<Args>...>) {
                func_ptr = new null_op_t(std::move([f = std::forward<Function>(func),
                                                    ... largs = std::forward<Args>(args),
                                                    token = stop_source_.get_token()] {
                    f(token, std::forward<Args>(largs)...);
                }));
            } else {
                func_ptr = null_op_t(std::move(
                    [f = std::forward<Function>(func), ... largs = std::forward<Args>(args)] {
                        f(std::forward<Args>(largs)...);
                    }));
            }
            pthread_create(
                &thread_, attr,
                [](void *callable) -> void * {
                    const std::function<void()> *f = static_cast<std::function<void()> *>(callable);
                    (*f)();
                    delete f;
                    return nullptr;
                },
                func_ptr);
        }

        // not copyable
        native_thread(const native_thread &) = delete;
        native_thread(native_thread &&) noexcept = default;

        native_thread &operator=(const native_thread &) = delete;
        native_thread &operator=(native_thread &&other) noexcept {
            if (this == std::addressof(other)) {
                return *this;
            }

            cancel_and_join();
            thread_ = std::exchange(other.thread_, 0);
            stop_source_ = std::move(other.stop_source_);
            return *this;
        }

        void join() {
            if (!joinable()) {
                throw std::runtime_error("Thread is not joinable");
            }
            const auto current_thread = pthread_self();
            if (pthread_equal(current_thread, thread_)) {
                throw std::runtime_error("Resource deadlock would occur.");
            }

            if (pthread_join(thread_, nullptr) != 0) {
                throw std::runtime_error("No such process.");
            }

            thread_ = 0;
        }

        void detach() {
            if (!joinable()) {
                throw std::runtime_error("Invalid argument.");
            }
            if (pthread_detach(thread_) == 0) {
                thread_ = 0;
            }
        }

        [[maybe_unused]] bool request_stop() noexcept {
            stop_source_.request_stop();
            return true;
        }
        [[nodiscard]] std::stop_source get_stop_source() const noexcept { return stop_source_; }
        [[nodiscard]] std::stop_token get_stop_token() const noexcept {
            return stop_source_.get_token();
        }

        [[nodiscard]] bool joinable() const noexcept { return thread_ != 0; }

      private:
        void cancel_and_join() {
            request_stop();
            join();
        }
        std::stop_source stop_source_;
        pthread_t thread_;
    };

    static_assert(details::JoinableThread<native_thread, std::function<void()>>);
#else
    using native_thread = std::jthread;
#endif
}  // namespace dp
