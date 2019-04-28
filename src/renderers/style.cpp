#include "renderers/style.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "scene/scene.h"
#include "renderers/styleiconengine.h"

namespace omm
{

Style::Style(Scene* scene)
  : m_scene(scene)
{
  add_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>").toStdString())
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property(PEN_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(true))
    .set_label(QObject::tr("active").toStdString())
    .set_category(QObject::tr("pen").toStdString());
  add_property(PEN_COLOR_KEY, std::make_unique<ColorProperty>(Colors::BLACK))
    .set_label(QObject::tr("color").toStdString())
    .set_category(QObject::tr("pen").toStdString());
  add_property(PEN_WIDTH_KEY, std::make_unique<FloatProperty>(1.0))
    .set_step(0.1)
    .set_range(0, std::numeric_limits<double>::infinity())
    .set_label(QObject::tr("width").toStdString())
    .set_category(QObject::tr("pen").toStdString());

  add_property(BRUSH_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(false))
    .set_label(QObject::tr("active").toStdString())
    .set_category(QObject::tr("brush").toStdString());
  add_property(BRUSH_COLOR_KEY, std::make_unique<ColorProperty>(Colors::RED))
    .set_label(QObject::tr("color").toStdString())
    .set_category(QObject::tr("brush").toStdString());
}

std::string Style::type() const { return TYPE; }
AbstractPropertyOwner::Flag Style::flags() const { return Flag::None; }
std::unique_ptr<Style> Style::clone() const
{
  auto clone = std::make_unique<Style>(m_scene);
  copy_properties(*clone);
  return clone;
}

QIcon Style::icon() const
{
  return QIcon(std::make_unique<StyleIconEngine>(*this).release());
}

SolidStyle::SolidStyle(const Color& color, Scene* scene) : Style(scene)
{
  property(omm::Style::PEN_IS_ACTIVE_KEY)->set(false);
  property(omm::Style::BRUSH_IS_ACTIVE_KEY)->set(true);
  property(omm::Style::BRUSH_COLOR_KEY)->set(color);
}

ContourStyle::ContourStyle(const Color& color, const double width, Scene* scene) : Style(scene)
{
  property(omm::Style::PEN_IS_ACTIVE_KEY)->set(true);
  property(omm::Style::BRUSH_IS_ACTIVE_KEY)->set(false);
  property(omm::Style::PEN_COLOR_KEY)->set(color);
  property(omm::Style::PEN_WIDTH_KEY)->set(width);
}

ContourStyle::ContourStyle(const Color& color, Scene* scene) : ContourStyle(color, 2.0, scene) { }

}  // namespace omm
