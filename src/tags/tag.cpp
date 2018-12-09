#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "serializers/jsonserializer.h"

namespace omm
{


Object* Tag::owner() const { return m_owner; }
void Tag::set_owner(Object* owner) { m_owner = owner; }

}  // namespace omm
