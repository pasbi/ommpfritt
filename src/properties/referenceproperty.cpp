#include "properties/referenceproperty.h"
#include "objects/object.h"
#include "serializers/abstractdeserializer.h"
#include "tags/tag.h"

namespace omm
{
const Property::PropertyDetail ReferenceProperty::detail{nullptr};

using Flag = Flag;
using Kind = Kind;

class ReferenceProperty::ReferencePolisher : public omm::serialization::ReferencePolisher
{
public:
  explicit ReferencePolisher(const std::size_t reference_value_id, ReferenceProperty& reference_property)
    : m_reference_value_id(reference_value_id)
    , m_reference_property(reference_property)
  {
  }

private:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& references) override
  {
    if (m_reference_value_id != 0) {
      try {
        m_reference_property.set(references.at(m_reference_value_id));
      } catch (const std::out_of_range&) {
        LWARNING << "Failed to restore reference for property " << m_reference_property.label();
      }
    }
  }

  std::size_t m_reference_value_id;
  ReferenceProperty& m_reference_property;
};

ReferenceProperty::ReferenceProperty() : TypedProperty(nullptr)
{
  configuration.set(FILTER_POINTER, PropertyFilter::accept_anything());
  ReferenceProperty::set_default_value(nullptr);
}

ReferenceProperty::ReferenceProperty(const ReferenceProperty& other)
    : TypedProperty<AbstractPropertyOwner*>(other)
{
  auto* value = this->value();
  if (value != nullptr) {
    ReferenceProperty::set(value);
  }
}

ReferenceProperty::~ReferenceProperty()
{
  QSignalBlocker blocker(this);
  ReferenceProperty::set(nullptr);
}

void ReferenceProperty::serialize(serialization::SerializerWorker& worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  configuration.get<PropertyFilter>(FILTER_POINTER).serialize(*worker.sub(ReferenceProperty::FILTER_POINTER));
}

void ReferenceProperty::deserialize(serialization::DeserializerWorker& worker)
{
  TypedProperty::deserialize(worker);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto reference_value_id = worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_size_t();
  {
    PropertyFilter filter;
    filter.deserialize(*worker.sub(ReferenceProperty::FILTER_POINTER));
    set_filter(filter);
  }
  worker.deserializer().register_reference_polisher(std::make_unique<ReferencePolisher>(reference_value_id, *this));
}

ReferenceProperty& ReferenceProperty::set_filter(const PropertyFilter& filter)
{
  configuration.set(FILTER_POINTER, filter);
  Q_EMIT this->configuration_changed();
  return *this;
}

void ReferenceProperty::set_default_value(const value_type& value)
{
  assert(value == nullptr);
  TypedProperty::set_default_value(value);
}

bool ReferenceProperty::is_compatible(const Property& other) const
{
  if (Property::is_compatible(other)) {
    const auto& other_reference_property = dynamic_cast<const ReferenceProperty&>(other);
    return other_reference_property.configuration.get(FILTER_POINTER)
           == configuration.get(FILTER_POINTER);
  } else {
    return false;
  }
}

PropertyFilter ReferenceProperty::filter() const
{
  return configuration.get<PropertyFilter>(ReferenceProperty::FILTER_POINTER);
}

void ReferenceProperty::revise()
{
  set(nullptr);
}

void ReferenceProperty::set(AbstractPropertyOwner* const& value)
{
  AbstractPropertyOwner* const old_value = this->value();
  TypedProperty::set(value);
  Q_EMIT reference_changed(old_value, value);
}

}  // namespace omm
