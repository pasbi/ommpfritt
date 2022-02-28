#pragma once

#include <QString>

namespace omm
{

class Scene;

namespace scene_serializer
{

enum class Format {JSON};
Format guess_format(const QString& filename);

}  // namespace scene_serializer

class SceneSerialization
{
public:
  explicit SceneSerialization(Scene& scene);

  template<typename Serializer> bool save(Serializer& serializer) const;
  bool save(const QString& filename) const;
  bool save_json(const QString& filename) const;
  bool save(const QString& filename, scene_serializer::Format format) const;

  template<typename Deserializer> bool load(Deserializer& deserializer) const;
  bool load(const QString& filename) const;
  bool load_json(const QString& filename) const;
  bool load(const QString& filename, scene_serializer::Format format) const;

private:
  Scene& m_scene;
};

}  // namespace
