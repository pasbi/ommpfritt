#include "gtest/gtest.h"

#include <cmath>
#include <QDebug>
#include "color/color.h"

namespace
{

const std::map<std::array<double, 3>, std::array<double, 3>> rgb_hsv {
  {{ 0.0, 0.0, 0.0 }, { 0, 0, 0 }},
  {{ 1.0, 1.0, 1.0 }, { 0, 0, 1 }},
  {{ 1.0, 0.0, 0.0 }, { 0.0 * M_PI/3.0, 1, 1 }},
  {{ 1.0, 1.0, 0.0 }, { 1.0 * M_PI/3.0, 1, 1 }},
  {{ 0.0, 1.0, 0.0 }, { 2.0 * M_PI/3.0, 1, 1 }},
  {{ 0.0, 1.0, 1.0 }, { 3.0 * M_PI/3.0, 1, 1 }},
  {{ 0.0, 0.0, 1.0 }, { 4.0 * M_PI/3.0, 1, 1 }},
  {{ 1.0, 0.0, 1.0 }, { 5.0 * M_PI/3.0, 1, 1 }},
  {{ 1.0, 0.1, 0.9 }, { 5.352343039449277, 0.9, 1.0 }},
  {{ 1.0, 0.8, 0.3 }, { 0.747998250854713, 0.7, 1.0 }},
  {{ 0.0, 0.2, 0.6 }, { 3.839724354387525, 1.0, 0.6 }},
  {{ 0.1, 0.4, 1.0 }, { 3.839724354387525, 0.9, 1.0 }},
  {{ 0.0, 0.9, 0.1 }, { 2.210750385859484, 1.0, 0.9 }},
  {{ 0.3, 0.3, 1.0 }, { 4.188790204786391, 0.7, 1.0 }},
  {{ 1.0, 0.0, 0.9 }, { 5.340707511102648, 1.0, 1.0 }},
};

const std::map<double, std::string> hex_lut {
  { 0.0, "00" },
  { 1.0, "ff" },
  { 0.1, "1a" },
  { 0.9, "e6" },
  { 0.8, "cc" },
  { 0.3, "4d" },
  { 0.2, "33" },
  { 0.6, "99" },
  { 0.4, "66" },
};

bool color_eq(const omm::Color& a, const omm::Color& b)
{
  const auto d = a - b;
  const double eps = 1.0/255.0;
  return std::abs(d.red()) < eps
      && std::abs(d.green()) < eps
      && std::abs(d.blue()) < eps
      && std::abs(d.alpha()) < eps;
}

}  // namespace

TEST(color, hsv_to_rgb)
{
  for (double v : { 1.0, 0.7, 0.1 }) {
    for (auto&& [ rgb, hsv ] : rgb_hsv) {
      const auto color = omm::Color::from_hsv(hsv[0], hsv[1], v * hsv[2], 1.0);
      EXPECT_FLOAT_EQ(color.red(), v * rgb[0]);
      EXPECT_FLOAT_EQ(color.green(), v * rgb[1]);
      EXPECT_FLOAT_EQ(color.blue(), v * rgb[2]);
    }
  }
}

TEST(color, rgb_to_hsv)
{
  for (double v : { 1.0, 0.7, 0.1 }) {
    for (auto&& [ rgb, hsv ] : rgb_hsv) {
      double hue, sat, val;
      omm::Color(v * rgb[0], v * rgb[1], v * rgb[2]).to_hsv(hue, sat, val);
      EXPECT_FLOAT_EQ(hue, hsv[0]);
      EXPECT_FLOAT_EQ(sat, hsv[1]);
      EXPECT_FLOAT_EQ(val, v * hsv[2]);
    }
  }
}

TEST(color, rgb_to_hex)
{
  for (auto&& [r, r_s] : hex_lut) {
    for (auto&& [g, g_s] : hex_lut) {
      for (auto&& [b, b_s] : hex_lut) {
        for (auto&& [a, a_s] : hex_lut) {
          const omm::Color color(r, g, b, a);
          EXPECT_EQ(color.to_hex(true), "#" + r_s + g_s + b_s);
          EXPECT_EQ(color.to_hex(false), "#" + r_s + g_s + b_s + a_s);
        }
      }
    }
  }
}

TEST(color, hex_to_rgb)
{
  for (auto&& [r, r_s] : hex_lut) {
    for (auto&& [g, g_s] : hex_lut) {
      for (auto&& [b, b_s] : hex_lut) {
        for (auto&& [a, a_s] : hex_lut) {
          EXPECT_TRUE(color_eq(omm::Color("#" + r_s + g_s + b_s), omm::Color(r, g, b)));
          EXPECT_TRUE(color_eq(omm::Color("#" + r_s + g_s + b_s + a_s), omm::Color(r, g, b, a)));
        }
      }
    }
  }
}
