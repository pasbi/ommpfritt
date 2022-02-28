#include "tags/tag.h"

#include <QObject>
#include <memory>

#include "external/json.hpp"
#include "objects/object.h"
#include "properties/stringproperty.h"
#include "scene/scene.h"

namespace omm
{
Tag::Tag(Object& owner) : PropertyOwner(owner.scene()), owner(&owner)
{
  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>"))
      .set_label(QObject::tr("Name"))
      .set_category(QObject::tr("basic"));
}

Tag::~Tag()
{
  assert(!owner->scene()->selection().contains(this));
}

QString Tag::to_string() const
{
  return QString{"%1[%2]"}.arg(type(), name());
}

Flag Tag::flags() const
{
  return Flag::None;
}

std::unique_ptr<Tag> Tag::clone(Object& owner) const
{
  auto clone = this->clone();
  clone->owner = &owner;
  return clone;
}

std::unique_ptr<Tag> Tag::clone() const
{
  return AbstractFactory::clone();
}

void Tag::force_evaluate()
{
  evaluate();
}

}  // namespace omm
