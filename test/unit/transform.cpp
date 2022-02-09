#include "transform.h"
#include "gtest/gtest.h"
#include <set>
#include <vector>
#include <deque>

TEST(TransformTest, explicit_value_type)
{
  std::set set{1, 2, 3, 4};
  const auto result = util::transform<double>(set);
  static_assert(std::is_same_v<decltype(result)::value_type, double>);
  EXPECT_EQ(result, (std::set{1.0, 2.0, 3.0, 4.0}));
}

TEST(TransformTest, explicit_container_and_functor)
{
  std::set set{1, 2, 3, 4};
  const auto result = util::transform<std::vector>(set, [](int i) { return i / 2.0; });
  static_assert(std::is_same_v<decltype(result)::value_type, double>);
  EXPECT_EQ(result, (std::vector{0.5, 1.0, 1.5, 2.0}));
}

TEST(TransformTest, explicit_container_type)
{
  std::set set{1, 2, 3, 4};
  const auto result = util::transform<std::vector>(set);
  static_assert(std::is_same_v<decltype(result)::value_type, int>);
  EXPECT_EQ(result, (std::vector{1, 2, 3, 4}));
}

TEST(TransformTest, explicit_container_and_value_type)
{
  std::set set{1, 2, 3, 4};
  const auto result = util::transform<double, std::vector>(set);
  static_assert(std::is_same_v<decltype(result)::value_type, double>);
  EXPECT_EQ(result, (std::vector{1.0, 2.0, 3.0, 4.0}));
}

TEST(TransformTest, explicit_functor)
{
  std::set set{1, 2, 3, 4};
  const auto result = util::transform(set, [](int i) { return i / 2.0; });
  static_assert(std::is_same_v<decltype(result)::value_type, double>);
  EXPECT_EQ(result, (std::set{0.5, 1.0, 1.5, 2.0}));
}

TEST(TransformTest, map)
{
  std::map<int, std::string> map{{1, "1"}, {2, "2"}, {3, "3"}};
  const auto keys = util::transform<std::vector>(map, [](const auto& key_value) { return key_value.first; });
  const auto values = util::transform<std::vector>(map, [](const auto& key_value) { return key_value.second; });
  static_assert(std::is_same_v<decltype(keys)::value_type, int>);
  static_assert(std::is_same_v<decltype(values)::value_type, std::string>);
  EXPECT_EQ(keys, (std::vector{1, 2, 3}));
  EXPECT_EQ(values, (std::vector<std::string>{"1", "2", "3"}));
}

TEST(TransformTest, array)
{
  std::array array{1, 2, 3, 4};
  const auto result = util::transform<std::vector>(array, [](int i) { return i / 2.0; });
  static_assert(std::is_same_v<decltype(result)::value_type, double>);
  EXPECT_EQ(result, (std::vector{0.5, 1.0, 1.5, 2.0}));
}

TEST(TransformTest, move)
{
  struct NoCopy
  {
    explicit NoCopy(int payload) : payload(payload) {}
    NoCopy(const NoCopy&) = delete;
    NoCopy(NoCopy&&) = default;
    NoCopy &operator=(const NoCopy&) = delete;
    NoCopy &operator=(NoCopy&&) = default;
    int payload;
    bool operator==(const NoCopy& other)
    {
      return payload == other.payload;
    }
  };

  std::vector<NoCopy> vector;
  vector.emplace_back(1);
  vector.emplace_back(2);
  vector.emplace_back(3);
  vector.emplace_back(4);
  const auto f = [](NoCopy&& v) -> decltype(auto) { return std::move(v); };
  auto deque = util::transform<std::deque>(std::move(vector), f);

  std::deque<NoCopy> expected_deque;
  expected_deque.emplace_back(1);
  expected_deque.emplace_back(2);
  expected_deque.emplace_back(3);
  expected_deque.emplace_back(4);
  EXPECT_EQ(deque, expected_deque);
}
