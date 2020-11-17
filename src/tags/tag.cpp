#include "tags/tag.h"

#include <QObject>
#include <memory>

#include "external/json.hpp"
#include "objects/object.h"
#include "properties/stringproperty.h"
#include "scene/scene.h"
#include "serializers/jsonserializer.h"

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
  assert(!::contains(owner->scene()->selection(), this));
}

std::ostream& operator<<(std::ostream& ostream, const Tag& tag)
{
  ostream << tag.type() << "[" << tag.name() << "]";
  return ostream;
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

}  // namespace omm
