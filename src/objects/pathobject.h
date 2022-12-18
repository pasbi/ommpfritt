#pragma once

#include "objects/object.h"
#include "config.h"
#include <memory>

namespace omm
{

class PathPoint;
class PathVector;
class Scene;
class Segment;

class PathObject : public Object
{
public:
  explicit PathObject(Scene* scene);
  explicit PathObject(Scene* scene, const PathVector& path_vector);
  explicit PathObject(Scene* scene, std::unique_ptr<PathVector> path_vector);
  PathObject(const PathObject& other);
  PathObject(PathObject&&) = delete;
  PathObject& operator=(PathObject&&) = delete;
  PathObject& operator=(const PathObject&) = delete;
  ~PathObject() override;
  QString type() const override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathObject");
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";
  static constexpr auto PATH_VECTOR_POINTER = "path-vector";

  void serialize(serialization::SerializerWorker& worker) const override;
  void deserialize(serialization::DeserializerWorker& worker) override;

  void on_property_value_changed(Property* property) override;
  Flag flags() const override;

  const PathVector& path_vector() const;
  PathVector& path_vector();

  std::unique_ptr<PathVector> compute_geometry() const override;
  void set_face_selected(const Face& face, bool s);
  [[nodiscard]] bool is_face_selected(const Face& face) const;

#if DRAW_POINT_IDS
  void draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const override;
#endif  // DRAW_POINT_IDS

private:
  std::unique_ptr<PathVector> m_path_vector;
};

}  // namespace omm
