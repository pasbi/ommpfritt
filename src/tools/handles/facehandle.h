#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"
#include "path/face.h"
#include <QPainterPath>

namespace omm
{
class FaceHandle : public Handle
{
public:
  explicit FaceHandle(Tool& tool, PathObject& path_object, const Face& face);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;
  Vec2f position = Vec2f::o();
  ObjectTransformation transformation() const;
  bool is_selected() const;

protected:
  bool transform_in_tool_space{};

private:
  PathObject& m_path_object;
  const Face m_face;
  const QPainterPath m_path;
};

}  // namespace omm
