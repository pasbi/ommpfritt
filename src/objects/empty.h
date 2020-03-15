#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{

class Empty : public Object
{
public:
  using Object::Object;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Empty");

};

std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
