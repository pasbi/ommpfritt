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

namespace omm
{

template<typename Deserializer> bool SceneSerialization::load(Deserializer& deserializer) const
{
  m_scene.reset();
  try {
    auto worker = deserializer.worker();
    m_scene.deserialize(*worker);
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
  auto worker = serializer.worker();
  m_scene.serialize(*worker);
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

  ofstream << json.dump(4) << "\n";
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
