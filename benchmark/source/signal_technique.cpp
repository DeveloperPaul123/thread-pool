#include <doctest/doctest.h>
#include <nanobench.h>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <semaphore>
#include <thread>

TEST_CASE("thread signaling techniques") {
    constexpr int count_limit = 1'000'000;

    using namespace std::chrono_literals;

    auto bench = ankerl::nanobench::Bench()
                     .title("Thread signaling")
                     .relative(true)
                     .output(&std::cout)
                     .minEpochIterations(10)
                     .timeUnit(1ms, "ms");

    bench.run("std::atomic_flag", [] {
        std::atomic_flag ping_flag = ATOMIC_FLAG_INIT;
        std::atomic_flag pong_flag = ATOMIC_FLAG_INIT;
        std::atomic<int> counter{};

        ping_flag.test_and_set();
        std::jthread ping_thread([&] {
            while (counter <= count_limit) {
                ping_flag.wait(false);
                ping_flag.clear();
                ++counter;
                pong_flag.test_and_set();
                pong_flag.notify_one();
            }
        });

        std::jthread pong_thread([&] {
            while (counter <= count_limit) {
                pong_flag.wait(false);
                pong_flag.clear();
                ping_flag.test_and_set();
                ping_flag.notify_one();
            }
        });
    });

    bench.run("std::binary_semaphore", [] {
        std::binary_semaphore ping_sem{0};
        std::binary_semaphore pong_sem{0};
        std::atomic<int> counter{};

        ping_sem.release();
        std::jthread ping_thread([&] {
            while (counter <= count_limit) {
                ping_sem.acquire();
                ++counter;
                pong_sem.release();
            }
        });

        std::jthread pong_thread([&] {
            while (counter <= count_limit) {
                pong_sem.acquire();
                ping_sem.release();
            }
        });
    });
}
