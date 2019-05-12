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
    .set_required_flags(AbstractPropertyOwner::Flag::IsPathLike)
    .set_label(QObject::tr("path").toStdString())
    .set_category(QObject::tr("path").toStdString());
  add_property<FloatProperty>(POSITION_PROPERTY_KEY)
    .set_step(0.001).set_range(0.0, 1.0)
    .set_label(QObject::tr("position").toStdString())
    .set_category(QObject::tr("path").toStdString());
  add_property<BoolProperty>(ALIGN_REFERENCE_PROPERTY_KEY)
    .set_label(QObject::tr("align").toStdString())
    .set_category(QObject::tr("path").toStdString());
}

QIcon PathTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward);
}

std::string PathTag::type() const { return TYPE; }
std::unique_ptr<Tag> PathTag::clone() const { return std::make_unique<PathTag>(*this); }
AbstractPropertyOwner::Flag PathTag::flags() const { return Tag::flags(); }

void PathTag::evaluate()
{
  auto* o = property(PATH_REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
  const double t = property(POSITION_PROPERTY_KEY)->value<double>();
  const bool align = property(ALIGN_REFERENCE_PROPERTY_KEY)->value<bool>();
  owner->set_position_on_path(o, align, t);
}

}  // namespace
