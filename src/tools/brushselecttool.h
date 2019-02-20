#pragma once

#include "tools/tool.h"
#include "tools/positionvariant.h"

namespace omm
{

class BrushSelectTool : public Tool
{
public:
  explicit BrushSelectTool(Scene& scene);
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TR_NOOP("BrushSelectTool");

  /**
   * @brief returns true if any grabbable was moved
   */
  bool mouse_move( const arma::vec2& delta, const arma::vec2& pos,
                           const QMouseEvent& event) override;

  /**
   * @brief returns true if any grabbable was hit
   */
  bool mouse_press(const arma::vec2& pos, const QMouseEvent& event) override;
  void mouse_release(const arma::vec2& pos, const QMouseEvent& event) override;
  std::string type() const override;
  void on_scene_changed() override;
  void on_selection_changed() override;
  PointPositions position_variant;
  void draw(AbstractRenderer& renderer) const override;
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";


private:
  bool m_mouse_down = false;
  arma::vec2 m_mouse_pos;
  const static Style m_style;
  void modify_selection(const arma::vec2& pos, const QMouseEvent& event);
};

void register_tools();

}  // namespace omm
