#include <doctest/doctest.h>
#include <thread_pool/thread_pool.h>

#include <string>

TEST_CASE("ThreadPool") {
  dp::thread_pool pool(2);
  // TODO
  auto future_value = pool.enqueue([](const int& value) { return value; }, 30);
  auto future_negative = pool.enqueue([](int x) -> int { return x - 20; }, 3);

  auto value1 = future_value.get();
  auto value2 = future_negative.get();
  CHECK(value1 == 30);
  CHECK(value2 == 3 - 20);
}
