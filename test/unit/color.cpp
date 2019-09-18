#include "gtest/gtest.h"

#include <cmath>
#include <QDebug>
#include "color/color.h"

namespace
{

const std::map<std::array<double, 3>, std::array<double, 3>> rgb_hsv {
  {{ 0, 0, 0 }, {0, 0, 0 }},
  {{ 1, 1, 1 }, {0, 0, 1 }},
  {{ 1, 0, 0 }, {0.0 * M_PI/3.0, 1, 1 }},
  {{ 1, 1, 0 }, {1.0 * M_PI/3.0, 1, 1 }},
  {{ 0, 1, 0 }, {2.0 * M_PI/3.0, 1, 1 }},
  {{ 0, 1, 1 }, {3.0 * M_PI/3.0, 1, 1 }},
  {{ 0, 0, 1 }, {4.0 * M_PI/3.0, 1, 1 }},
  {{ 1, 0, 1 }, {5.0 * M_PI/3.0, 1, 1 }},
  {{ 1.0, 0.1, 0.9 }, { 5.352343039449277, 0.9, 1.0 }},
  {{ 1.0, 0.8, 0.3 }, { 0.747998250854713, 0.7, 1.0 }},
  {{ 0.0, 0.2, 0.6 }, { 3.839724354387525, 1.0, 0.6 }},
  {{ 0.1, 0.4, 1.0 }, { 3.839724354387525, 0.9, 1.0 }},
  {{ 0.0, 0.9, 0.1 }, { 2.210750385859484, 1.0, 0.9 }},
  {{ 0.3, 0.3, 1.0 }, { 4.188790204786391, 0.7, 1.0 }},
  {{ 1.0, 0.0, 0.9 }, { 5.340707511102648, 1.0, 1.0 }},
  };

  }

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
