#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "serializers/jsonserializer.h"

namespace omm
{

Tag::Tag(Object& owner) : owner(&owner)
{
  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>").toStdString())
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());
}

std::ostream& operator<<(std::ostream& ostream, const Tag& tag)
{
  ostream << tag.type() << "[" << tag.name() << "]";
  return ostream;  return ostream;
}

AbstractPropertyOwner::Flag Tag::flags() const { return Flag::None; }

std::unique_ptr<Tag> Tag::clone(Object& owner) const
{
  auto clone = this->clone();
  clone->owner = &owner;
  return clone;
}

}  // namespace omm
