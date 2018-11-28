#include "scene/stylepool.h"

namespace
{

std::unique_ptr<omm::Style> make_default_style()
{
  auto default_style = std::make_unique<omm::Style>();
  default_style->is_pen_active = true;
  default_style->pen_color = omm::Color::BLACK;
  default_style->pen_width = 2.0;
  default_style->is_brush_active = true;
  default_style->brush_color = omm::Color::BLUE;
  return default_style;
}

}  // namespace

namespace omm
{

StylePool::StylePool()
  : m_default_style(make_default_style())
{

}

const Style& StylePool::default_style() const
{
  return *m_default_style;
}

std::set<Style*> StylePool::styles() const
{
  return ::transform<Style*, std::set>(m_styles, [](const auto& up) { return up.get(); });
}

}  // namespace omm
