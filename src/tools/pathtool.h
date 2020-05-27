#pragma once

#include "tools/selecttool.h"
#include "objects/path.h"
#include "tools/selectpointstool.h"
#include <memory>

namespace omm
{

class PathTool : public SelectPointsBaseTool
{
public:
  explicit PathTool(Scene& scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTool");
  bool mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &event) override;
  bool mouse_press(const Vec2f &pos, const QMouseEvent &event) override;
  void mouse_release(const Vec2f &pos, const QMouseEvent &event) override;
  QString type() const override;
  void end() override;
  void reset() override;

private:
  Path::iterator add_point(const Vec2f& pos);
  Path* m_path = nullptr;
  std::optional<Path::iterator> m_current_point;

};

}  // namespace
