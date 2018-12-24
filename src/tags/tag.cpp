#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "serializers/jsonserializer.h"

namespace omm
{

Tag::Tag(Object& owner) : owner(owner) {}

std::unique_ptr<Tag> Tag::copy() const
{
  return Copyable<Tag>::copy(this->make(this->type(), owner));
}

}  // namespace omm
