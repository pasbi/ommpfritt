#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{
class Empty : public Object
{
public:
  explicit Empty(Scene* scene);
  static constexpr auto JOIN_PROPERTY_KEY = "join";
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Empty");
  PathVector compute_path_vector() const override;
  Flag flags() const override;
};

}  // namespace omm
