#pragma once

#include "tools/tool.h"
#include "objects/path.h"
#include <memory>

namespace omm
{

class PathTool : public Tool
{
public:
  explicit PathTool(Scene& scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "PathTool");
  bool mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &event) override;
  bool mouse_press(const Vec2f &pos, const QMouseEvent &event, bool force = false) override;
  void mouse_release(const Vec2f &pos, const QMouseEvent &event) override;
  std::string type() const override;
  void cancel() override;
  void draw(AbstractRenderer& renderer) const override;
  void end() override;

private:
  void add_point(const Vec2f& pos);

  std::unique_ptr<Path> m_path;
  Point* m_current_point = nullptr;

};

}  // namespace
