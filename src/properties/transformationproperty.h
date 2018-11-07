#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class TransformationProperty : public TypedProperty<ObjectTransformation>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  std::string widget_type() const override;
};

}  // namespace omm
