#pragma once

#include "objects/object.h"
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

  const PathVector& geometry() const;
  PathVector& geometry();

  PathVector compute_path_vector() const override;

#ifdef DRAW_POINT_IDS
  void draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const override;
#endif  // DRAW_POINT_IDS

private:
  std::unique_ptr<PathVector> m_path_vector;
};

}  // namespace omm
