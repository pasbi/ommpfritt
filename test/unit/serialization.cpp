#include "config.h"
#include "external/json.hpp"
#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "python/pythonengine.h"
#include "scene/scene.h"
#include "scene/sceneserializer.h"
#include "serializers/json/jsondeserializer.h"
#include "serializers/json/jsonserializer.h"
#include "testutil.h"
#include <fstream>

namespace
{

std::unique_ptr<omm::Options> options()
{
  return std::make_unique<omm::Options>(false, // is_cli
                                        false  // have_opengl
  );
}

bool scene_eq(const nlohmann::json& a, const nlohmann::json& b)
{
  if (a.type() != b.type()) {
    return false;
  }

//  const std::set<std::string> excluded_keys = ""

  switch (a.type()) {
  case nlohmann::detail::value_t::array:
    if (a.size() != b.size()) {
      return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
      if (!scene_eq(a.at(i), b.at(i))) {
        return false;
      }
    }
  case nlohmann::detail::value_t::object:
    if (a.size() != b.size()) {
      return false;
    }
    for (const auto& [key, value] : a.items()) {
      if (!b.contains(key) || !scene_eq(value, b.at(key))) {
        return false;
      }
    }
  case nlohmann::detail::value_t::number_float:
    static constexpr auto eps = 0.00001;
    return (static_cast<double>(a) - static_cast<double>(b)) < eps;
  default:
    return a == b;
  }
  return true;
}

}  // namespace

TEST(serialization, JSON)
{
  QStringList test_files{
    "icons/icons.omm",
  };
  for (const auto& fn : test_files) {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    std::ifstream ifstream{abs_fn.toStdString()};

    omm::Application app(ommtest::qt_gui_app->gui_application(), options());

    nlohmann::json json_file;
    ifstream >> json_file;
    omm::serialization::JSONDeserializer deserializer(json_file);
    omm::SceneSerialization{*app.scene}.load(deserializer);

    nlohmann::json store;
    omm::serialization::JSONSerializer serializer(store);
    omm::SceneSerialization{*app.scene}.save(serializer);
    if (scene_eq(json_file, store)) {
      const auto diff = nlohmann::json::diff(json_file, store);
      LINFO << "diff: " << QString::fromStdString(diff.dump(2));
      EXPECT_TRUE(scene_eq(json_file, store));
    }
  }
}
