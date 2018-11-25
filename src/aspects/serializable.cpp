#include "aspects/serializable.h"

#include <functional>
#include "serializers/jsonserializer.h"

namespace omm
{

Serializable::IdType Serializable::identify() const
{
  return std::hash<const void*>()(this);
}

}  // namespace omm
