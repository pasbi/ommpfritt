#include "config.h"
#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "mainwindow/exporter.h"
#include "objects/view.h"
#include "scene/scene.h"
#include "testutil.h"
#include "transform.h"
#include <QImage>
#include <map>
#include <vector>

namespace
{

class NonUniqueException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

auto options()
{
  return std::make_unique<omm::Options>(false, false);
}

auto& find_unique_item(omm::Scene& scene, const QString& name)
{
  const auto items = scene.find_items<omm::Object>(name);
  if (items.size() != 1) {
    throw NonUniqueException{""};
  }
  return **items.begin();
}

QImage render_icon(omm::Scene& scene)
{
  static constexpr QSize size{256, 256};
  static constexpr double scale = 1.0;

  omm::Exporter exporter{scene};
  exporter.export_options.x_resolution = size.width();
  exporter.export_options.view = &type_cast<omm::View&>(find_unique_item(scene, "view"));
  exporter.y_resolution = size.height();

  QImage image{size, QImage::Format_ARGB32_Premultiplied};
  image.fill(Qt::transparent);
  exporter.render(image, scale);
  return image;
}

auto compute_histogram(const QImage& image)
{
  static constexpr auto color_less = [](const auto& a, const auto b) { return a.rgba64() < b.rgba64(); };
  std::map<QColor, std::size_t, decltype(color_less)> counts;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      const auto c = image.pixelColor(x, y);
      if (const auto [it, new_color] = counts.try_emplace(c, 0); !new_color) {
        it->second += 1;
      }
    }
  }
  return counts;
}

struct IconTestParameter
{
  QString scene_filename;
  QString object_name;
  std::vector<QColor> expected_colors;
};

class Icon : public ::testing::TestWithParam<IconTestParameter>
{
protected:
  Icon()
    : m_app(ommtest::Application(options()))
    , m_scene(*m_app.omm_app().scene)
  {
  }

  void prepare_scene(const QString& filename, const QString& object_name)
  {
    ASSERT_TRUE(m_scene.load_from(filename));
    auto& p = *find_unique_item(m_scene, object_name).property(omm::Object::VISIBILITY_PROPERTY_KEY);
    p.set(omm::Object::Visibility::Visible);
  }

  std::vector<QColor> find_dominant_colors(const std::size_t n) const
  {
    const auto image = ::render_icon(m_scene);
    const auto histogram = ::compute_histogram(image);

    std::vector<std::pair<QColor, std::size_t>> colors(n, {{}, 0});
    static constexpr auto oftener = [](const auto& a, const auto& b) { return a.second > b.second; };
    std::partial_sort_copy(histogram.begin(), histogram.end(), colors.begin(), colors.end(), oftener);
    static constexpr auto first = [](const auto& pair) { return pair.first; };
    return util::transform(colors, first);
  }

private:
  ommtest::Application m_app;
  omm::Scene& m_scene;
};

}  // namespace

TEST_P(Icon, CheckDominantColors)
{
  const auto& param = GetParam();
  prepare_scene(QString{source_directory} + "/" + param.scene_filename, param.object_name);
  const auto dominant_colors = find_dominant_colors(param.expected_colors.size());

  for (std::size_t i = 0; i < dominant_colors.size(); ++i) {
    const auto actual = dominant_colors.at(i);
    const auto expected = param.expected_colors.at(i);
    EXPECT_EQ(actual, expected) << "expected " << expected.name(QColor::HexArgb).toStdString()
                                << " but got " << actual.name(QColor::HexArgb).toStdString();
  }
}

static constexpr QColor omm_red{202, 0, 47, 255};
static constexpr QColor no_color{0, 0, 0, 0};
static constexpr QColor omm_yellow{238, 238, 0, 255};
static constexpr QColor black{0, 0, 0, 255};
static constexpr QColor light_green{70, 255, 122, 255};
static constexpr QColor strong_green{47, 141, 31, 255};

INSTANTIATE_TEST_SUITE_P(Icon, Icon, testing::Values(
  IconTestParameter{"icons/icons.omm", "omm", {no_color, omm_red, omm_yellow, black}},
  IconTestParameter{"icons/icons.omm", "Mirror", {no_color, strong_green, light_green}}
));
