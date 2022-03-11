#pragma once

#include <QString>

namespace omm
{

class Scene;

namespace scene_serializer
{

enum class Format {JSON, Binary};
Format guess_format(const QString& filename);

}  // namespace scene_serializer

class SceneSerialization
{
public:
  explicit SceneSerialization(Scene& scene);

  template<typename Serializer> bool save(Serializer& serializer) const;  // NOLINT(modernize-use-nodiscard)
  bool save(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool save_json(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool save_bin(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool save(const QString& filename, scene_serializer::Format format) const;  // NOLINT(modernize-use-nodiscard)

  template<typename Deserializer> bool load(Deserializer& deserializer) const;  // NOLINT(modernize-use-nodiscard)
  bool load(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool load_json(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool load_bin(const QString& filename) const;  // NOLINT(modernize-use-nodiscard)
  bool load(const QString& filename, scene_serializer::Format format) const;  // NOLINT(modernize-use-nodiscard)

private:
  Scene& m_scene;
};

}  // namespace
