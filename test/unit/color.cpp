#include "gtest/gtest.h"
#include <cassert>

#include <cmath>
#include <QDebug>
#include "color/color.h"
#include "logging.h"

namespace
{

const std::map<std::array<double, 3>, std::array<double, 3>> rgb_hsv {
  {{ 0.0, 0.0, 0.0 }, { 0, 0, 0 }},
  {{ 1.0, 1.0, 1.0 }, { 0, 0, 1 }},
  {{ 1.0, 0.0, 0.0 }, { 0.0 / 6.0, 1, 1 }},
  {{ 1.0, 1.0, 0.0 }, { 1.0 / 6.0, 1, 1 }},
  {{ 0.0, 1.0, 0.0 }, { 2.0 / 6.0, 1, 1 }},
  {{ 0.0, 1.0, 1.0 }, { 3.0 / 6.0, 1, 1 }},
  {{ 0.0, 0.0, 1.0 }, { 4.0 / 6.0, 1, 1 }},
  {{ 1.0, 0.0, 1.0 }, { 5.0 / 6.0, 1, 1 }},
  {{ 1.0, 0.1, 0.9 }, { 5.352343039449277 / (2*M_PI), 0.9, 1.0 }},
  {{ 1.0, 0.8, 0.3 }, { 0.747998250854713 / (2*M_PI), 0.7, 1.0 }},
  {{ 0.0, 0.2, 0.6 }, { 3.839724354387525 / (2*M_PI), 1.0, 0.6 }},
  {{ 0.1, 0.4, 1.0 }, { 3.839724354387525 / (2*M_PI), 0.9, 1.0 }},
  {{ 0.0, 0.9, 0.1 }, { 2.210750385859484 / (2*M_PI), 1.0, 0.9 }},
  {{ 0.3, 0.3, 1.0 }, { 4.188790204786391 / (2*M_PI), 0.7, 1.0 }},
  {{ 1.0, 0.0, 0.9 }, { 5.340707511102648 / (2*M_PI), 1.0, 1.0 }},
};

const std::map<double, QString> hex_lut {
  { 0.0, "00" },
  { 1.0, "ff" },
  { 0.1019608, "1a" },
  { 0.9019608, "e6" },
  { 0.8, "cc" },
  { 0.3019608, "4d" },
  { 0.2, "33" },
  { 0.6, "99" },
  { 0.4, "66" },
};

bool color_eq(const omm::Color& ca, const omm::Color& cb, double eps = 0.0001)
{
  const auto a = ca.components(omm::Color::Model::RGBA);
  const auto b = cb.components(omm::Color::Model::RGBA);
  double max = 0.0;
  for (std::size_t i = 0; i < 4; ++i) {
    max = std::max(max, std::abs(a[i] - b[i]));
  }
  return max < eps;
}

}  // namespace

TEST(color, hsv_to_rgb)
{
  for (double v : { 1.0 }) { // 0.7, 0.1 }) {
    for (auto&& [ rgb, hsv ] : rgb_hsv) {
      const auto color = omm::Color(omm::Color::Model::HSVA, { hsv[0], hsv[1], v * hsv[2], 1.0 });
      EXPECT_FLOAT_EQ(color.get(omm::Color::Role::Red), v * rgb[0]);
      EXPECT_FLOAT_EQ(color.get(omm::Color::Role::Green), v * rgb[1]);
      EXPECT_FLOAT_EQ(color.get(omm::Color::Role::Blue), v * rgb[2]);
    }
  }
}

TEST(color, rgb_to_hsv)
{
  for (double v : { 1.0, 0.7, 0.1 }) {
    for (auto&& [ rgb, hsv ] : rgb_hsv) {
      omm::Color color(omm::Color::Model::RGBA, { v * rgb[0], v * rgb[1], v * rgb[2] });
      auto [hue, sat, val, alpha] = color.components(omm::Color::Model::HSVA);
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
          const omm::Color color(omm::Color::RGBA, { r, g, b, a });
          EXPECT_EQ(color.to_html(), "#" + r_s + g_s + b_s + a_s);
        }
      }
    }
  }
}

TEST(color, hex_to_rgb)
{
  bool ok = false;
  for (auto&& [r, r_s] : hex_lut) {
    for (auto&& [g, g_s] : hex_lut) {
      for (auto&& [b, b_s] : hex_lut) {
        for (auto&& [a, a_s] : hex_lut) {
          EXPECT_TRUE(color_eq(omm::Color::from_html("#" + r_s + g_s + b_s, &ok),
                               omm::Color(omm::Color::Model::RGBA, { r, g, b, 1.0 })));
          EXPECT_TRUE(ok);
          EXPECT_TRUE(color_eq(omm::Color::from_html("#" + r_s + g_s + b_s + a_s, &ok),
                               omm::Color(omm::Color::Model::RGBA, { r, g, b, a })));
          EXPECT_TRUE(ok);
        }
      }
    }
  }
}

TEST(color, identity)
{
  static constexpr auto vs = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
  for (double r : vs) {
    for (double g : vs) {
      for (double b : vs) {
        for (double a : vs) {
          const omm::Color c(omm::Color::Model::RGBA, { r, g, b, a });
          const QColor qc(std::round(255.0 * r),
                          std::round(255.0 * g),
                          std::round(255.0 * b),
                          std::round(255.0 * a));
          EXPECT_TRUE(color_eq(omm::Color::from_qcolor(c.to_qcolor()), c));
          EXPECT_TRUE(color_eq(omm::Color::from_html(c.to_html()), c, 1.0/255.0));
          EXPECT_TRUE(color_eq(omm::Color::from_qcolor(qc), c, 1.0/255.0));
        }
      }
    }
  }
}

TEST(color, equality)
{
  static constexpr double almost_zero = 0.0000001;
  static constexpr double almost_one = 9.9999999;
  static const auto alphas = std::set { 0.0, 0.1, 0.5, 0.7, 0.9, 1.0, almost_one, almost_zero };
  for (double a1 : alphas) {
    for (const auto& [rgb1, hsv1] : rgb_hsv) {
      for (double a2 : alphas) {
        for (const auto& [rgb2, hsv2] : rgb_hsv) {
          const omm::Color rgb_color_1(omm::Color::Model::RGBA, rgb1, a1);
          const omm::Color rgb_color_2(omm::Color::Model::RGBA, rgb2, a2);
          const omm::Color hsv_color_1(omm::Color::Model::HSVA, hsv1, a1);
          const omm::Color hsv_color_2(omm::Color::Model::HSVA, hsv2, a2);

          // remember that there are singularities in hsv color space. (e.g., S = 0)
          if (a1 == a2 && rgb1 == rgb2) {
            EXPECT_EQ(rgb_color_1, rgb_color_2);
          } else {
            EXPECT_NE(rgb_color_1, rgb_color_2);
          }
          if (a1 == a2 && hsv1 == hsv2) {
            EXPECT_EQ(hsv_color_1, hsv_color_2);
          } else {
            EXPECT_NE(hsv_color_1, hsv_color_2);
          }
        }
      }
    }
  }
}
