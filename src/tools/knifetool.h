#pragma once

#include "tools/selecttool.h"

namespace omm
{

class KnifeTool : public SelectPointsTool
{
public:
  explicit KnifeTool(Scene& scene);
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "KnifeTool");

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent&) override;
  bool mouse_press(const arma::vec2& pos, const QMouseEvent& event, bool force) override;
  void mouse_release(const arma::vec2& pos, const QMouseEvent& event) override;
  std::string type() const override;
  void draw(AbstractRenderer& renderer) const override;
  std::string name() const override;

private:
  bool m_is_cutting = false;
  arma::vec2 m_mouse_press_pos;
  arma::vec2 m_mouse_move_pos;
  static const Style m_line_style;
  std::list<Point> m_points;
};

}  // namespace omm
