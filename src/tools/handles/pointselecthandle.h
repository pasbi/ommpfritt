#pragma once

#include "tools/handles/abstractselecthandle.h"
#include "tools/handles/tangenthandle.h"
#include "geometry/objecttransformation.h"
#include <memory>

class QMouseEvent;

namespace omm
{

class PathObject;
class PathPoint;

class PointSelectHandle : public AbstractSelectHandle
{
public:
  enum class TangentMode { Mirror, Individual };
  explicit PointSelectHandle(Tool& tool, PathObject& path_object, PathPoint& point);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  [[nodiscard]] PathPoint& point() const;
  void transform_tangent(const Vec2f& delta, const Point::TangentKey& tangent_key);

protected:
  [[nodiscard]] ObjectTransformation transformation() const;
  void set_selected(bool selected) override;
  void clear() override;
  [[nodiscard]] bool is_selected() const override;

private:
  PathObject& m_path_object;
  PathPoint& m_point;
  std::map<Point::TangentKey, std::unique_ptr<TangentHandle>> m_tangent_handles;
  [[nodiscard]] std::pair<bool, bool> tangents_active() const;
  std::map<Point::TangentKey, PolarCoordinates> other_tangents(const Point::TangentKey& tangent_key) const;
  bool is_active(const Point::TangentKey& tangent_key) const;

  void transform_tangent(const Vec2f& delta, TangentMode mode, const Point::TangentKey& tangent_key);
};

}  // namespace
