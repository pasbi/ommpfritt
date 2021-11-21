#pragma once

#include "tools/selectpointstool.h"
#include <memory>

namespace omm
{

class PathTool : public SelectPointsBaseTool
{
public:
  explicit PathTool(Scene& scene);
  ~PathTool() override;
  PathTool(const PathTool&) = delete;
  PathTool(PathTool&&) = delete;
  PathTool operator=(const PathTool&) = delete;
  PathTool operator=(PathTool&&) = delete;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTool");
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  [[nodiscard]] QString type() const override;
  void end() override;
  void reset() override;
  struct Current;

private:
  std::unique_ptr<Current> m_current;
};

}  // namespace omm
