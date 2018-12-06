#include "renderers/style.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "scene/scene.h"

namespace omm
{

Style::Style(Scene* scene) : m_scene(scene)
{
  add_property(PEN_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(false))
    .set_label("active").set_category("pen");
  add_property(PEN_COLOR_KEY, std::make_unique<ColorProperty>(Color::BLACK))
    .set_label("color").set_category("pen");
  add_property(PEN_WIDTH_KEY, std::make_unique<FloatProperty>(1.0))
    .set_label("width").set_category("pen");

  add_property(BRUSH_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(false))
    .set_label("active").set_category("brush");
  add_property(BRUSH_COLOR_KEY, std::make_unique<ColorProperty>(Color::RED))
    .set_label("color").set_category("brush");
}

size_t Style::row() const
{
  assert(m_scene != nullptr);
  return m_scene->position(*this);
}

std::unique_ptr<Style> Style::copy() const
{
  return Copyable<Style>::copy(std::make_unique<Style>(m_scene));
}

const Style* Style::predecessor() const
{
  const auto row = this->row();
  if (row == 0) {
    return nullptr;
  } else {
    return &m_scene->style(row - 1);
  }
}

}  // namespace omm
