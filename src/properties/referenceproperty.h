#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class Object;

class ReferenceProperty : public TypedProperty<Object*>
{
public:
  ReferenceProperty();
  static bool is_referenced(const Object* candidate);
  void set_value(Object* value) override;
  std::string type() const override;
  std::string widget_type() const override;

private:
  static std::unordered_set<const Object*> m_references;
};

}  // namespace omm
