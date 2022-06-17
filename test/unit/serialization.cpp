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

TEST(serialization, JSONInvalidScene)
{
  ommtest::Application qt_app;
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));
}

TEST(serialization, BinaryInvalidScene)
{
  ommtest::Application qt_app;
  nlohmann::json json_file;
  omm::serialization::JSONDeserializer deserializer(json_file);
  EXPECT_FALSE(omm::SceneSerialization{*qt_app.omm_app().scene}.load(deserializer));
}

template<typename Buffer>
class BufferSerialization : public testing::Test
{
protected:
  Buffer buffer;
};

using BufferTypes = ::testing::Types<BinaryBuffer, JSONBuffer>;
TYPED_TEST_SUITE(BufferSerialization, BufferTypes);

TYPED_TEST(BufferSerialization, Property)
{
  const QString default_value = "foo";
  omm::StringProperty property(default_value);
  // only user properties are fully serialized and can be tested easily.
  property.set_category(omm::Property::USER_PROPERTY_CATEGROY_NAME).set_label("foo-label");
  property.set(QString{"bar"});

  this->buffer.serialize([&property](auto& serializer) {
    property.serialize(*serializer.worker());
  });

  omm::StringProperty other_property;
  this->buffer.deserialize([&other_property](auto& deserializer) {
    other_property.deserialize(*deserializer.worker());
  });

  EXPECT_EQ(property.value(), other_property.value());
  EXPECT_EQ(property.default_value(), other_property.default_value());
}

TYPED_TEST(BufferSerialization, Object)
{
  omm::Ellipse ellipse(nullptr);

  this->buffer.serialize([&ellipse](auto& serializer) {
    ellipse.serialize(*serializer.worker());
  });

  omm::Ellipse other_ellipse(nullptr);
  this->buffer.deserialize([&other_ellipse](auto& deserializer) {
    other_ellipse.deserialize(*deserializer.worker());
  });

  EXPECT_TRUE(ellipse.eq(other_ellipse));
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

class SceneFromFileInvariance : public testing::TestWithParam<QString>
{
protected:
  SceneFromFileInvariance()
      : m_scene(*m_app.omm_app().scene)
  {
  }

  bool test_json_serialization(const QString& fn)
  {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    LINFO << "loading " << abs_fn;
    return load_json(abs_fn) && save_json() && compare();
  }

  bool test_binary_serialization(const QString& fn)
  {
    const auto abs_fn = QString{source_directory} + "/" + fn;
    LINFO << "loading " << abs_fn;

    QByteArray buffer;
    return load_json(abs_fn) && save_bin(buffer) && load_bin(buffer) && save_json() && compare();
  }

  std::string reason() const
  {
    return m_reason;
  }

  bool load_json(const QString& filename)
  {
    std::ifstream ifstream{filename.toStdString()};
    ifstream >> m_expected;
    omm::serialization::JSONDeserializer deserializer(m_expected);
    if (!omm::SceneSerialization{m_scene}.load(deserializer)) {
      m_reason = "JSON Deserialization failed.";
      return false;
    }
    return true;
  }

  bool save_json()
  {
    omm::serialization::JSONSerializer serializer(m_actual);
    if (!omm::SceneSerialization{m_scene}.save(serializer)) {
      m_reason = "JSON Serialization failed.";
      return false;
    }
    return true;
  }

  bool load_bin(const QByteArray& buffer)
  {
    QDataStream deserialize_stream{buffer};
    omm::serialization::BinDeserializer bin_deserializer(deserialize_stream);
    if (!omm::SceneSerialization{m_scene}.load(bin_deserializer)) {
      m_reason = "Binary deserialization failed.";
      return false;
    }
    return true;
  }

  bool save_bin(QByteArray& buffer)
  {
    QDataStream serialize_stream{&buffer, QIODevice::WriteOnly};
    omm::serialization::BinSerializer bin_serializer(serialize_stream);
    if (!omm::SceneSerialization{m_scene}.save(bin_serializer)) {
      m_reason = "Binary serialization failed.";
      return false;
    }
    return true;
  }

  bool compare()
  {
    if (!scene_eq(m_expected, m_actual)) {
      m_reason = "Serializing after Deserialization is not identity: ";
      m_reason += "Expected: " + m_expected.dump(4) + "\n\n";
      m_reason += "Actual: " + m_actual.dump(4) + "\n\n";
      m_reason += "Diff: " + nlohmann::json::diff(m_expected, m_actual).dump(4);
      return false;
    }

    return true;
  }

private:
  ommtest::Application m_app;
  omm::Scene& m_scene;
  nlohmann::json m_expected;
  nlohmann::json m_actual;
  std::string m_reason;
};

TEST_P(SceneFromFileInvariance, JSON)
{
  EXPECT_TRUE(test_json_serialization(GetParam())) << reason();
}

TEST_P(SceneFromFileInvariance, Binary)
{
  EXPECT_TRUE(test_binary_serialization(GetParam())) << reason();
}

INSTANTIATE_TEST_SUITE_P(Serialization, SceneFromFileInvariance, testing::Values(
    "sample-scenes/basic.omm",
    "sample-scenes/animation.omm",
    "sample-scenes/python.omm",
    "sample-scenes/glshader.omm",
    "sample-scenes/nodes.omm",
    "icons/icons.omm"
));
