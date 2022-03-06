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
#include "serializers/bin/bindeserializer.h"
#include "serializers/bin/binserializer.h"
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

QStringList test_files()
{
  return {
    "sample-scenes/basic.omm",
    "sample-scenes/animation.omm",
    "sample-scenes/python.omm",
    "sample-scenes/glshader.omm",
    "sample-scenes/nodes.omm",
    "icons/icons.omm",
  };
}

}  // namespace

TEST(serialization, JSON)
{
  for (const auto& fn : static_cast<const QStringList>(test_files())) {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    std::ifstream ifstream{abs_fn.toStdString()};

    auto app = ommtest::qt_gui_app->make_application(options());

    nlohmann::json json_file;
    ifstream >> json_file;
    omm::serialization::JSONDeserializer deserializer(json_file);
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.load(deserializer));

    app->scene->reset();

    nlohmann::json store;
    omm::serialization::JSONSerializer serializer(store);
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.save(serializer));
    if (scene_eq(json_file, store)) {
      const auto diff = nlohmann::json::diff(json_file, store);
      LINFO << "diff: " << QString::fromStdString(diff.dump(2));
      EXPECT_TRUE(scene_eq(json_file, store));
    }
  }

  auto app = ommtest::qt_gui_app->make_application(options());
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*app->scene}.load(deserializer));
}

TEST(serialization, Binary)
{
  for (const auto& fn : static_cast<const QStringList>(test_files())) {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    std::ifstream ifstream{abs_fn.toStdString()};

    auto app = ommtest::qt_gui_app->make_application(options());

    nlohmann::json json_file;
    ifstream >> json_file;
    omm::serialization::JSONDeserializer deserializer(json_file);
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.load(deserializer));

    QByteArray buffer;
    QDataStream serialize_stream{&buffer, QIODevice::WriteOnly};
    omm::serialization::BinSerializer bin_serializer(serialize_stream);
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.save(bin_serializer));

    app->scene->reset();

    QDataStream deserialize_stream{buffer};
    omm::serialization::BinDeserializer bin_deserializer(deserialize_stream);
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.load(bin_deserializer));

    nlohmann::json store;
    omm::serialization::JSONSerializer serializer{store};
    EXPECT_TRUE(omm::SceneSerialization{*app->scene}.save(serializer));

    if (scene_eq(json_file, store)) {
      const auto diff = nlohmann::json::diff(json_file, store);
      LINFO << "diff: " << QString::fromStdString(diff.dump(2));
      EXPECT_TRUE(scene_eq(json_file, store));
    }
  }

  auto app = ommtest::qt_gui_app->make_application(options());
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*app->scene}.load(deserializer));
}
