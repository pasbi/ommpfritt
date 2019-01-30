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
  add_property<StringProperty>(NAME_PROPERTY_KEY, "<unnamed object>")
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());
}

std::ostream& operator<<(std::ostream& ostream, const Tag& tag)
{
  ostream << tag.type() << "[" << tag.name() << "]";
  return ostream;  return ostream;
}

}  // namespace omm
