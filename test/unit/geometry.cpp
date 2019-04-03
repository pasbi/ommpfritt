#include <gtest/gtest.h>
#include <glog/logging.h>
#include <random>
#include "geometry/objecttransformation.h"
#include "logging.h"

namespace
{

double mod(double a, double b)
{
  assert(b > 0);
  const double mod = fmod(fmod(a, b) + b, b);
  assert(mod >= 0.0);
  assert(mod < b);
  return mod;
}

bool fuzzy_equal(omm::ObjectTransformation a, omm::ObjectTransformation b)
{
  a = a.normalized();
  b = b.normalized();

  const auto fuzzy_equal = [](double a, double b, double abs_eps) {
    return abs(a-b) <= abs_eps;
  };
  const auto normalize_radians = [](double rad){ return mod(rad, 2*M_PI); };

  static constexpr auto rad_eps = 0.0001;
  static constexpr auto trans_eps = 0.0001;
  static constexpr auto scale_eps = 0.0001;
  static constexpr auto shear_eps = 0.0001;

  if (!fuzzy_equal(normalize_radians(a.rotation()), normalize_radians(b.rotation()), rad_eps)) {
    return false;
  } else if (!fuzzy_equal(a.translation().x, b.translation().x, trans_eps)) {
    return false;
  } else if (!fuzzy_equal(a.translation().y, b.translation().y, trans_eps)) {
    return false;
  } else if (!fuzzy_equal(a.scaling().x, b.scaling().x, scale_eps)) {
    return false;
  } else if (!fuzzy_equal(a.scaling().y, b.scaling().y, scale_eps)) {
    return false;
  } else if (!fuzzy_equal(a.shearing(), b.shearing(), shear_eps)) {
    return false;
  } else {
    return true;
  }
}

template<typename Arg> using SetParameterF = void(omm::ObjectTransformation::*)(const Arg& arg);
template<typename Arg>
bool check_transform_to_mat_to_transform_invariant(SetParameterF<Arg> f, const Arg& arg)
{
  omm::ObjectTransformation reference;
  (reference.*f)(arg);

  const auto other = omm::ObjectTransformation(reference.to_mat());

  if (fuzzy_equal(reference, other)) {
    // LOG(INFO) << "pass " << typeid(f).name() << " [" << arg << "]";
    return true;
  } else {
    LINFO << "expected: \n" << reference;
    LINFO << "but got: \n" << other;
    // LOG(WARNING) << "fail " << typeid(f).name() << " [" << arg << "]";
    return false;
  }
}

auto translation_test_cases()
{
  using v = omm::Vec2f;
  return std::vector<v>({ v(0.0, 0.0), v(1.0, 0.0), v(0.0, 1.0), v(-1.0, 0.0), v(0.0, -1.0),
                          v(3, -4), v(12, -909), v(-1232, -22), v(73, 73), v(0.012, -0.778) });
}

auto rotation_test_cases()
{
  return std::vector<double>({ 0.0, 1.0, -1.0, 0.5*M_PI, -0.5*M_PI, 0.25*M_PI, -0.25*M_PI,
                               0.6*M_PI });
  //0.0, M_PI, 2*M_PI, 0.5*M_PI, 0.25*M_PI, 1, 2, 100,
                               //-M_PI, -2*M_PI, -0.5*M_PI, -0.25*M_PI, -1, -2, -0.75*M_PI, 100 });
}

auto scaling_test_cases()
{
  using v = omm::Vec2f;
  return std::vector<v>({ v(1, 1), v(3, -4), v(12, -909), v(-1232, -22), v(73, 73),
                          v(0.012, -0.778) });
}

auto shearing_test_cases()
{
  return std::vector<double>({ 0.0, 1, -1, 2, -2, 100, -100, 1.2324, -32.3435 });
}

}  // namespace

TEST(geometry, transform_to_from_mat_translate)
{
  for (auto translation : translation_test_cases()) {
    EXPECT_TRUE(check_transform_to_mat_to_transform_invariant(
      &omm::ObjectTransformation::set_translation, translation ) );
  }
}

TEST(geometry, transform_to_from_mat_rotate)
{
  for (auto rotation : rotation_test_cases()) {
    EXPECT_TRUE(check_transform_to_mat_to_transform_invariant(
      &omm::ObjectTransformation::set_rotation, rotation ) );
  }
}

TEST(geometry, transform_to_from_mat_scale)
{
  for (auto scaling : scaling_test_cases()) {
    EXPECT_TRUE(check_transform_to_mat_to_transform_invariant(
      &omm::ObjectTransformation::set_scaling, scaling ) );
  }
}

TEST(geometry, transform_to_from_mat_shear)
{
  for (auto shearing : shearing_test_cases()) {
    EXPECT_TRUE(check_transform_to_mat_to_transform_invariant(
      &omm::ObjectTransformation::set_shearing, shearing ) );
  }
}

TEST(geometry, transform_to_from_mat_combined)
{
  for (auto translation : translation_test_cases()) {
    for (auto rotation : rotation_test_cases()) {
      for (auto scaling : scaling_test_cases()) {
        for (auto shearing : shearing_test_cases()) {
          omm::ObjectTransformation first_order;
          first_order.set_rotation(rotation);
          first_order.set_translation(translation);
          first_order.set_scaling(scaling);
          first_order.set_shearing(shearing);
          EXPECT_TRUE(fuzzy_equal(first_order, omm::ObjectTransformation(first_order.to_mat())));

          omm::ObjectTransformation second_order;
          second_order.set_translation(translation);
          second_order.set_rotation(rotation);
          second_order.set_scaling(scaling);
          second_order.set_shearing(shearing);
          EXPECT_TRUE(fuzzy_equal(second_order, omm::ObjectTransformation(second_order.to_mat())));

          omm::ObjectTransformation third_order;
          third_order.set_translation(translation);
          third_order.set_scaling(scaling);
          third_order.set_shearing(shearing);
          third_order.set_rotation(rotation);
          EXPECT_TRUE(fuzzy_equal(third_order, omm::ObjectTransformation(third_order.to_mat())));
        }
      }
    }
  }
}

TEST(geometry, transform_to_from_mat_random)
{
  std::mt19937 rng;
  rng.seed(42);
  std::uniform_real_distribution<> distribution(-100, 100);
  constexpr auto n = 10000;

  for (size_t i = 0; i < n; ++i) {
    omm::ObjectTransformation t;
    t.set_translation({ distribution(rng), distribution(rng) });
    t.set_scaling({ distribution(rng), distribution(rng) });
    t.set_shearing(distribution(rng));
    t.set_rotation(distribution(rng));
    EXPECT_TRUE(fuzzy_equal(t, omm::ObjectTransformation(t.to_mat())));
  }

}
