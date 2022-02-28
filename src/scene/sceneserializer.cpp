#include "sceneserializer.h"

#include "animation/animator.h"
#include "color/namedcolors.h"
#include "history/historymodel.h"
#include "mainwindow/exportoptions.h"
#include "renderers/style.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "scene/stylelist.h"
#include "scene/disjointpathpointsetforest.h"

#include <QDataStream>
#include <QFile>
#include <fstream>

#include "serializers/json/jsondeserializer.h"
#include "serializers/json/jsonserializer.h"

namespace
{

constexpr auto ROOT_POINTER = "root";
constexpr auto STYLES_POINTER = "styles";
constexpr auto ANIMATOR_POINTER = "animation";
constexpr auto NAMED_COLORS_POINTER = "colors";
constexpr auto EXPORT_OPTIONS_POINTER = "export_options";
constexpr auto JOINED_POINTS_POINTER =  "joined_points";

}  // namespace exportoptions

namespace omm
{

template<typename Deserializer> bool SceneSerialization::load(Deserializer& deserializer) const
{
  m_scene.reset();
  try {
    auto new_root = m_scene.make_root();
    new_root->deserialize(*deserializer.sub(ROOT_POINTER));

    std::deque<std::unique_ptr<Style>> styles;
    deserializer.sub(STYLES_POINTER)->get_items([this, &styles](auto& worker_i) {
      auto style = std::make_unique<Style>(&m_scene);
      style->deserialize(worker_i);
      styles.push_back(std::move(style));
    });

    m_scene.history().set_saved_index();

    m_scene.object_tree().replace_root(std::move(new_root));
    m_scene.styles().set(std::move(styles));
    m_scene.animator().invalidate();

    m_scene.object_tree().root().update_recursive();

    m_scene.animator().deserialize(*deserializer.sub(ANIMATOR_POINTER));
    m_scene.named_colors().deserialize(*deserializer.sub(NAMED_COLORS_POINTER));
    m_scene.m_export_options->deserialize(*deserializer.sub(EXPORT_OPTIONS_POINTER));
    m_scene.m_joined_points->deserialize(*deserializer.sub(JOINED_POINTS_POINTER));
  } catch (const Object::AbstractFactory::InvalidKeyError& e) {
    LERROR << "Failed to deserialize file: " << e.what();
    return false;
  } catch (const serialization::AbstractDeserializer::DeserializeError& e) {
    LERROR << "Failed to deserialize file: " << e.what();
    return false;
  }

  deserializer.polish();
  return true;
}

template<typename Serializer> bool SceneSerialization::save(Serializer& serializer) const
{
  m_scene.object_tree().root().serialize(*serializer.sub(ROOT_POINTER));

  serializer.sub(STYLES_POINTER)->set_value(m_scene.styles().items());

  m_scene.m_animator->serialize(*serializer.sub(ANIMATOR_POINTER));
  m_scene.m_named_colors->serialize(*serializer.sub(NAMED_COLORS_POINTER));
  m_scene.export_options().serialize(*serializer.sub(EXPORT_OPTIONS_POINTER));
  m_scene.m_joined_points->serialize(*serializer.sub(JOINED_POINTS_POINTER));
  return true;
}

bool SceneSerialization::load(const QString& filename) const
{
  const auto format = scene_serializer::guess_format(filename);
  if (load(filename, format)) {
    m_scene.m_filename = filename;
    Q_EMIT m_scene.mail_box().filename_changed();
    return true;
  } else {
    return false;
  }
}

SceneSerialization::SceneSerialization(Scene& scene)
    : m_scene(scene)
{
}

bool SceneSerialization::save(const QString& filename) const
{
  const auto format = scene_serializer::guess_format(filename);
  if (save(filename, format)) {
    LINFO << "Saved current scene to '" << filename << "'.";
    m_scene.history().set_saved_index();
    m_scene.m_has_pending_changes = false;
    m_scene.m_filename = filename;
    Q_EMIT m_scene.mail_box().filename_changed();
    return true;
  } else {
    return false;
  }
}

bool SceneSerialization::load(const QString& filename, scene_serializer::Format format) const
{
  using scene_serializer::Format;
  switch (format) {
  case Format::JSON:
    return load_json(filename);
  }
  LERROR << "Cannot deserialize from unexpected format: " << static_cast<int>(format);
  return false;
}

bool SceneSerialization::load_json(const QString& filename) const
{
  std::ifstream ifstream(filename.toStdString());
  if (!ifstream) {
    LERROR << "Failed to open '" << filename << "'.";
    return false;
  }

  nlohmann::json json;
  try {
    ifstream >> json;
  } catch (const nlohmann::json::exception& e) {
    LERROR << "Failed to parse JSON file '" << filename << "': " << e.what();
    return false;
  }
  serialization::JSONDeserializer deserializer{json};
  return load(deserializer);
}

bool SceneSerialization::save_json(const QString& filename) const
{
  std::ofstream ofstream(filename.toStdString());
  if (!ofstream) {
    LERROR << "Failed to open ofstream at '" << filename << "'.";
    return false;
  }

  nlohmann::json json;
  serialization::JSONSerializer serializer{json};
  if (!save(serializer)) {
    return false;
  }

  ofstream << json.dump(4);
  return true;
}

bool SceneSerialization::save(const QString& filename, scene_serializer::Format format) const
{
  using scene_serializer::Format;
  switch (format) {
  case Format::JSON:
    return save_json(filename);
  }
  LERROR << "Cannot serialize to unexpected format: " << static_cast<int>(format);
  return false;
}

scene_serializer::Format scene_serializer::guess_format(const QString& filename)
{
  Q_UNUSED(filename)
  return Format::JSON;
}


}  // namespace omm
