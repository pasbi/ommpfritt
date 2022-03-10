#include "config.h"
#include "external/json.hpp"
#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "objects/ellipse.h"
#include "properties/stringproperty.h"
#include "python/pythonengine.h"
#include "scene/scene.h"
#include "scene/sceneserializer.h"
#include "serializers/bin/bindeserializer.h"
#include "serializers/bin/binserializer.h"
#include "serializers/json/jsondeserializer.h"
#include "serializers/json/jsonserializer.h"
#include "testutil.h"
#include <QFile>
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
  static constexpr auto object_t = nlohmann::detail::value_t::object;
  if (a.type() != object_t && b.type() != object_t) {
    return false;
  }
  const auto diff = nlohmann::json::diff(a, b);
  const auto operation_is_numerically_negligible = [&a, &b](const auto d) {
    if (d["op"] != "replace") {
      return false;
    }
    const nlohmann::json::json_pointer pointer{static_cast<std::string>(d["path"])};
    const auto& v_a = a.at(pointer);
    const auto& v_b = b.at(pointer);
    static constexpr auto float_t = nlohmann::detail::value_t::number_float;
    if (v_a.type() != float_t || v_b.type() != float_t) {
      return false;
    }
    static constexpr auto eps = 0.0001;
    return std::abs(static_cast<double>(v_a) - static_cast<double>(v_b)) < eps;
  };
  return std::all_of(diff.begin(), diff.end(), operation_is_numerically_negligible);
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

class BinaryBuffer
{
public:
  template<typename F> void serialize(F&& f)
  {
    QDataStream ostream{&m_buffer, QIODevice::WriteOnly};
    omm::serialization::BinSerializer serializer{ostream};
    std::forward<F>(f)(serializer);
  }

  template<typename F> void deserialize(F&& f)
  {
    omm::StringProperty other_property;
    QDataStream istream{&m_buffer, QIODevice::ReadOnly};
    omm::serialization::BinDeserializer deserializer{istream};
    std::forward<F>(f)(deserializer);
  }

  void write(const QString& filename) const
  {
    QFile file{filename};
    file.open(QIODevice::WriteOnly);
    file.write(m_buffer);
  }

private:
  QByteArray m_buffer;
};

class JSONBuffer
{
public:
  template<typename F> void serialize(F&& f)
  {
    omm::serialization::JSONSerializer serializer{m_store};
    std::forward<F>(f)(serializer);

    std::cout << m_store.dump(2) << std::endl;
  }

  template<typename F> void deserialize(F&& f)
  {
    omm::serialization::JSONDeserializer deserializer{m_store};
    std::forward<F>(f)(deserializer);
  }

  void write(const QString& filename) const
  {
    std::ofstream ostr{filename.toStdString()};
    ostr << m_store.dump(2);
  }

private:
  nlohmann::json m_store;
};

template<typename Buffer> void test_property_serialization()
{
  const QString default_value = "foo";
  omm::StringProperty property(default_value);
  // only user properties are fully serialized and can be tested easily.
  property.set_category(omm::Property::USER_PROPERTY_CATEGROY_NAME).set_label("foo-label");
  property.set(QString{"bar"});

  Buffer buffer;
  buffer.serialize([&property](auto& serializer) {
    property.serialize(*serializer.worker());
  });

  omm::StringProperty other_property;
  buffer.deserialize([&other_property](auto& deserializer) {
    other_property.deserialize(*deserializer.worker());
  });

  EXPECT_EQ(property.value(), other_property.value());
  EXPECT_EQ(property.default_value(), other_property.default_value());
}

template<typename Buffer> void test_object_serialization()
{
  omm::Ellipse ellipse(nullptr);

  Buffer buffer;
  buffer.serialize([&ellipse](auto& serializer) {
    ellipse.serialize(*serializer.worker());
  });

  buffer.write("/tmp/foo.bin");

  omm::Ellipse other_ellipse(nullptr);
  buffer.deserialize([&other_ellipse](auto& deserializer) {
    other_ellipse.deserialize(*deserializer.worker());
  });

  EXPECT_TRUE(ellipse.eq(other_ellipse));
}

}  // namespace

TEST(serialization, SceneEq)
{
  nlohmann::json a;
  nlohmann::json b;

  a = {{"key", "value"}};
  EXPECT_FALSE(scene_eq(a, b));
  EXPECT_TRUE(scene_eq(a, a));

  a = {{"key", {1.0, "a", 2}}};
  b = {{"key", {}}};
  EXPECT_FALSE(scene_eq(a, b));
  EXPECT_TRUE(scene_eq(a, a));
  EXPECT_TRUE(scene_eq(b, b));

  a = {{"key", {1.0, {{"sub", {}}}, 2}}};
  b = {{"key", {1.0, {{"sub", 0.0}}, 2}}};
  EXPECT_FALSE(scene_eq(a, b));
  EXPECT_TRUE(scene_eq(a, a));
  EXPECT_TRUE(scene_eq(b, b));

  a = {{"key", {1.0, {{"sub", 1.23456789}}, 2}}};
  b = {{"key", {1.0, {{"sub", 1.23456588}}, 2}}};
  EXPECT_TRUE(scene_eq(a, b));
  EXPECT_TRUE(scene_eq(a, a));
  EXPECT_TRUE(scene_eq(b, b));
}

TEST(serialization, JSONScene)
{
  for (const auto& fn : static_cast<const QStringList>(test_files())) {
    LINFO << "loading " << fn;
    const auto abs_fn = QString{source_directory} + "/" + fn;

    ommtest::Application qt_app{options()};

    std::ifstream ifstream{abs_fn.toStdString()};
    nlohmann::json json_file;
    ifstream >> json_file;
    omm::serialization::JSONDeserializer deserializer(json_file);
    app->scene->reset();
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));

    nlohmann::json store;
    omm::serialization::JSONSerializer serializer(store);
    if (scene_eq(json_file, store)) {
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.save(serializer));
      const auto diff = nlohmann::json::diff(json_file, store);
      LINFO << "diff: " << QString::fromStdString(diff.dump(2));
      LINFO << "store: " << QString::fromStdString(store.dump(4));
      EXPECT_TRUE(scene_eq(json_file, store));
    }
  }

  ommtest::Application qt_app{options()};
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));
}

TEST(serialization, BinaryScene)
{
  for (const auto& fn : static_cast<const QStringList>(test_files())) {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    std::ifstream ifstream{abs_fn.toStdString()};

    ommtest::Application qt_app{options()};

    nlohmann::json json_file;
    ifstream >> json_file;
    omm::serialization::JSONDeserializer deserializer(json_file);
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));

    QByteArray buffer;
    QDataStream serialize_stream{&buffer, QIODevice::WriteOnly};
    omm::serialization::BinSerializer bin_serializer(serialize_stream);
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.save(bin_serializer));

    app->scene->reset();

    QDataStream deserialize_stream{buffer};
    omm::serialization::BinDeserializer bin_deserializer(deserialize_stream);
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(bin_deserializer));

    nlohmann::json store;
    omm::serialization::JSONSerializer serializer{store};
    EXPECT_TRUE(omm::SceneSerialization{*qt_app.omm_app().scene}.save(serializer));

    if (scene_eq(json_file, store)) {
      const auto diff = nlohmann::json::diff(json_file, store);
      LINFO << "diff: " << QString::fromStdString(diff.dump(2));
      EXPECT_TRUE(scene_eq(json_file, store));
    }
  }

  ommtest::Application qt_app{options()};
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));
}

TEST(serialization, BinaryProperty)
{
  test_property_serialization<BinaryBuffer>();
}

TEST(serialization, JSONProperty)
{
  test_property_serialization<JSONBuffer>();
}

TEST(serialization, BinaryObject)
{
  test_object_serialization<BinaryBuffer>();
}

TEST(serialization, JSONObject)
{
  test_object_serialization<JSONBuffer>();
}

TEST(serialization, BinaryString)
{
  const QString value = "foobar";

  QByteArray buffer;
  QDataStream ostream{&buffer, QIODevice::WriteOnly};
  ostream << quint64{1039847};
  ostream << value;

  QString other_value;
  quint64 x;
  QDataStream istream{&buffer, QIODevice::ReadOnly};
  istream >> x;
  istream >> other_value;

  EXPECT_EQ(value, other_value);
}
