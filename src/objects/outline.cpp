#include "objects/outline.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "renderers/style.h"

namespace omm
{

Outline::Outline(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Outline").toStdString();
  create_property<FloatProperty>(OFFSET_PROPERTY_KEY)
    .set_label(QObject::tr("Offset").toStdString()).set_category(category);
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label(QObject::tr("Reference").toStdString()).set_category(category);
  update();
}

Outline::Outline(const Outline &other)
  : Object(other), m_outline(PathUniquePtr(new Path(*other.m_outline)))
{ }

void Outline::draw_object(Painter &renderer, const Style& style) const
{
  assert(&renderer.scene == scene());
  if (m_outline) {
    LINFO << style.property(Style::PEN_COLOR_KEY)->value<Color>();
    LINFO << style.property(Style::PEN_WIDTH_KEY)->value<double>();
    LINFO << renderer.current_transformation();
    m_outline->draw_recursive(renderer, style);
  }
}

BoundingBox Outline::bounding_box(const ObjectTransformation &transformation) const
{
  return m_outline ? m_outline->bounding_box(transformation) : BoundingBox();
}

std::string Outline::type() const { return TYPE; }
std::unique_ptr<Object> Outline::clone() const { return std::make_unique<Outline>(*this); }
AbstractPropertyOwner::Flag Outline::flags() const
{
  return Object::flags() | Flag::Convertable | Flag::IsPathLike; }

void Outline::update()
{
  if (is_active()) {
    const auto* ref = property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
    const auto t = property(OFFSET_PROPERTY_KEY)->value<double>();
    if (auto* o = kind_cast<const Object*>(ref); o != nullptr) {
      m_outline = o->outline(t);
      LINFO << m_outline->points();
    } else {
      m_outline.reset();
    }
  } else {
    m_outline.reset();
  }
  Object::update();
}

Point Outline::evaluate(const double t) const
{
  if (m_outline) {
    return m_outline->evaluate(t);
  } else {
    return Point();
  }
}

double Outline::path_length() const
{
  if (m_outline) {
    return m_outline->path_length();
  } else {
    return 0.0;
  }
}

bool Outline::contains(const Vec2f &pos) const
{
  auto* ref = kind_cast<Object*>(property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>());
  if (ref) {ref->update(); }

  if (m_outline) {
    return m_outline->contains(pos);
  } else {
    return false;
  }
}

void Outline::on_property_value_changed(Property *property)
{
  if (   property == this->property(OFFSET_PROPERTY_KEY)
      || property == this->property(REFERENCE_PROPERTY_KEY))
  {
    Q_EMIT appearance_changed(this);
  } else {
    Object::on_property_value_changed(property);
  }
}

std::unique_ptr<Object> Outline::convert() const
{
  auto converted = m_outline->clone();
  copy_properties(*converted);
  copy_tags(*converted);
  converted->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
  return std::unique_ptr<Object>(converted.release());
}


}  // namespace omm
