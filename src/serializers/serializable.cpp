#include "serializers/jsonserializer.h"

#include <functional>

#include "serializers/serializable.h"

namespace omm
{

Serializable::IdType Serializable::identify() const
{
  return std::hash<const void*>()(this);
}

}  // namespace omm
