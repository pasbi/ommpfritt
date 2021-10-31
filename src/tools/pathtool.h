#pragma once

#include "tools/selectpointstool.h"
#include <memory>

namespace omm
{

class Path;
class Segment;

class PathTool : public SelectPointsBaseTool
{
public:
  explicit PathTool(Scene& scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTool");
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  QString type() const override;
  void end() override;
  void reset() override;

private:
  Path* m_current_path = nullptr;
  Point* m_last_point = nullptr;
  Point* m_current_point = nullptr;
  Segment* m_current_segment = nullptr;
  void find_tie();
};

}  // namespace omm
