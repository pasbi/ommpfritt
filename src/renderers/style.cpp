#include "renderers/style.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "scene/scene.h"
#include "renderers/styleiconengine.h"
#include "properties/optionsproperty.h"

namespace omm
{

Style::Style(Scene* scene)
  : m_scene(scene)
{
  const auto pen_category = QObject::tr("pen").toStdString();
  const auto brush_category = QObject::tr("brush").toStdString();
  add_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>").toStdString())
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property(PEN_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(true))
    .set_label(QObject::tr("active").toStdString())
    .set_category(pen_category);
  add_property(PEN_COLOR_KEY, std::make_unique<ColorProperty>(Colors::BLACK))
    .set_label(QObject::tr("color").toStdString())
    .set_category(pen_category);
  add_property(PEN_WIDTH_KEY, std::make_unique<FloatProperty>(1.0))
    .set_step(0.1)
    .set_range(0, std::numeric_limits<double>::infinity())
    .set_label(QObject::tr("width").toStdString())
    .set_category(pen_category);
  add_property<OptionsProperty>(STROKE_STYLE_KEY)
    .set_options({ QObject::tr("Solid").toStdString(),
                   QObject::tr("Dashed").toStdString(),
                   QObject::tr("Dotted").toStdString(),
                   QObject::tr("DashDotted").toStdString(),
                   QObject::tr("DashDotDotted").toStdString() })
    .set_label(QObject::tr("Stroke Style").toStdString()).set_category(pen_category);
  add_property<OptionsProperty>(JOIN_STYLE_KEY)
    .set_options({ QObject::tr("Bevel").toStdString(),
                   QObject::tr("Miter").toStdString(),
                   QObject::tr("Round").toStdString() })
    .set_label(QObject::tr("Join").toStdString()).set_category(pen_category);
  add_property<OptionsProperty>(CAP_STYLE_KEY)
    .set_options({ QObject::tr("Square").toStdString(),
                   QObject::tr("Flat").toStdString(),
                   QObject::tr("Round").toStdString() })
    .set_label(QObject::tr("Cap").toStdString()).set_category(pen_category);
  add_property<BoolProperty>(COSMETIC_KEY, true).set_label(QObject::tr("Cosmetic").toStdString())
      .set_category(pen_category);

  add_property(BRUSH_IS_ACTIVE_KEY, std::make_unique<BoolProperty>(false))
    .set_label(QObject::tr("active").toStdString())
    .set_category(brush_category);
  add_property(BRUSH_COLOR_KEY, std::make_unique<ColorProperty>(Colors::RED))
    .set_label(QObject::tr("color").toStdString())
    .set_category(brush_category);
}

std::string Style::type() const { return TYPE; }
AbstractPropertyOwner::Flag Style::flags() const { return Flag::None; }
Scene *Style::scene() const { return m_scene; }

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
