#include "gtest/gtest.h"
#include "geometry/cubic.h"

TEST(cubic, find_roots)
{
  std::srand(42);
  for (int i = 0; i < 1000; ++i) {
    std::array<double, 3> roots;
    for (std::size_t j = 0; j < std::size_t(3); ++j) {
      const double t = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
      roots[j] = 200.0 * t - 100.0;
    }

    std::array<double, 3> coefficients = {
      -(roots[0] + roots[1] + roots[2]),
      roots[0]*roots[1] + roots[0]*roots[2] + roots[2]*roots[1],
      -roots[0]*roots[1]*roots[2]
    };

    auto the_roots = omm::find_cubic_roots(coefficients);
    std::sort(the_roots.begin(), the_roots.end());
    std::sort(roots.begin(), roots.end());

    ASSERT_EQ(the_roots.size(), roots.size());
    for (std::size_t j = 0; j < std::size_t(3); ++j) {
      EXPECT_LE(std::abs(the_roots[j] - roots[j]), 10e-10);
    }
  }
}
