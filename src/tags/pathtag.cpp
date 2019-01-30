#include "tags/pathtag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include "properties/referenceproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "objects/object.h"

namespace omm
{

PathTag::PathTag(Object& owner) : Tag(owner)
{
  add_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(AbstractPropertyOwner::Kind::Object)
    .set_label("path").set_category("path");
  add_property<FloatProperty>(POSITION_PROPERTY_KEY)
    .set_label("position").set_category("path");
  add_property<BoolProperty>(ALIGN_REFERENCE_PROPERTY_KEY)
    .set_label("align").set_category("path");
}

QIcon PathTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward);
}

std::string PathTag::type() const { return TYPE; }
std::unique_ptr<Tag> PathTag::clone() const { return std::make_unique<PathTag>(*this); }

void PathTag::evaluate()
{
  auto* o = property(PATH_REFERENCE_PROPERTY_KEY).value<AbstractPropertyOwner*>();
  const double t = property(POSITION_PROPERTY_KEY).value<double>();
  const bool align = property(ALIGN_REFERENCE_PROPERTY_KEY).value<bool>();
  owner->set_position_on_path(o, align, t);
}

}  // namespace
