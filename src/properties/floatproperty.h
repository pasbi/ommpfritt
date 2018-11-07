#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class FloatProperty : public TypedProperty<double>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  std::string widget_type() const override;
};

}  // namespace omm
