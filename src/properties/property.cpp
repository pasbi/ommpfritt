#include "properties/property.h"
#include <algorithm>
#include <assert.h>
#include "objects/object.h"

#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "properties/transformationproperty.h"

namespace omm
{

Property::Property()
{
}

Property::~Property()
{

}

std::string Property::label() const
{
  return m_label;
}

std::string Property::category() const
{
  return m_category;
}

Property& Property::set_label(const std::string& label)
{
  m_label = label;
  return *this;
}

Property& Property::set_category(const std::string& category)
{
  m_category = category;
  return *this;
}

void Property::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Serializable::serialize(serializer, root);
  serializer.set_value(m_label, make_pointer(root, "label"));
  serializer.set_value(m_category, make_pointer(root, "category"));
}

void Property
::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Serializable::deserialize(deserializer, root);
  m_label = deserializer.get_string(make_pointer(root, "label"));
  m_category = deserializer.get_string(make_pointer( root, "category" ));
}

std::string Property::get_label(const SetOfProperties& properties)
{
  assert(properties.size() > 0);
  const auto label = (*properties.begin())->label();
#ifndef NDEBUG
  for (const auto& property : properties) {
    if (property->label() != label) {
      LOG(WARNING) << "Inconsistent label: '" << label << "' != '" << property->label() << "'.";
    }
  }
#endif
  return label;
}

}  // namespace omm
