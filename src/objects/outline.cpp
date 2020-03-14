#include "objects/outline.h"

#include <QObject>
#include "objects/empty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "renderers/style.h"
#include "scene/scene.h"

namespace omm
{

Outline::Outline(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Outline");
  create_property<FloatProperty>(OFFSET_PROPERTY_KEY)
    .set_label(QObject::tr("Offset")).set_category(category);
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_filter(ReferenceProperty::Filter({ Kind::Object },
                                            DNF<Flag>{ { Flag::IsPathLike }}))
    .set_label(QObject::tr("Reference")).set_category(category);
  polish();
}

Outline::Outline(const Outline &other)
  : Object(other), m_outline(other.m_outline ? std::make_unique<Path>(*other.m_outline) : nullptr)
{
  polish();
}

void Outline::polish()
{
  listen_to_changes([this]() {
    return kind_cast<Object*>(property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>());
  });
  update();
}

void Outline::draw_object(Painter &renderer, const Style& style, Painter::Options options) const
{
  assert(&renderer.scene == scene());
  options.default_style = &style;
  if (m_outline) {
    m_outline->draw_recursive(renderer, options);
  }
}

BoundingBox Outline::bounding_box(const ObjectTransformation &transformation) const
{
  return m_outline ? m_outline->bounding_box(transformation) : BoundingBox();
}

QString Outline::type() const { return TYPE; }
std::unique_ptr<Object> Outline::clone() const { return std::make_unique<Outline>(*this); }
Flag Outline::flags() const
{
  return Object::flags() | Flag::Convertible | Flag::IsPathLike; }

void Outline::update()
{
  // scope of the cycle_guard object must be the whole update-function body!
  auto cycle_guard = scene()->make_cycle_guard(this);
  if (cycle_guard->inside_cycle()) {
    return;
  }

  if (is_active()) {
    const auto* ref = property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
    const auto t = property(OFFSET_PROPERTY_KEY)->value<double>();
    if (auto* o = kind_cast<const Object*>(ref); o != nullptr) {
      m_outline = o->outline(t);
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
    LINFO << "update outline: " << this;
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

std::unique_ptr<Object> Outline::convert() const
{
  auto converted = m_outline->clone();
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);
  converted->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
  return std::unique_ptr<Object>(converted.release());
}


}  // namespace omm
