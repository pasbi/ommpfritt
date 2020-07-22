#include "properties/referenceproperty.h"
#include "objects/object.h"
#include "tags/tag.h"

namespace omm
{

const Property::PropertyDetail ReferenceProperty::detail { nullptr };

using Flag = Flag;
using Kind = Kind;

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
{
  configuration[FILTER_POINTER] = Filter::accept_anything();
  set_default_value(nullptr);
}

ReferenceProperty::ReferenceProperty(const ReferenceProperty &other)
  : TypedProperty<AbstractPropertyOwner *>(other)
{
  auto* value = this->value();
  if (value != nullptr) {
    set(value);
  }
}

ReferenceProperty::~ReferenceProperty()
{
  QSignalBlocker blocker(this);
  set(nullptr);
}

void ReferenceProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value(configuration.get<Filter>(FILTER_POINTER),
                       make_pointer(root, ReferenceProperty::FILTER_POINTER));
}

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto ref_pointer = make_pointer(root, TypedPropertyDetail::VALUE_POINTER);
  m_reference_value_id = deserializer.get_size_t(ref_pointer);
  {
    Filter filter;
    deserializer.get(filter, make_pointer(root, ReferenceProperty::FILTER_POINTER));
    set_filter(filter);
  }
  deserializer.register_reference_polisher(*this);
}

ReferenceProperty& ReferenceProperty::set_filter(const ReferenceProperty::Filter& filter)
{
  configuration[FILTER_POINTER] = filter;
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
    auto& other_reference_property = static_cast<const ReferenceProperty&>(other);
    return other_reference_property.configuration.at(FILTER_POINTER) == configuration.at(FILTER_POINTER);
  } else {
    return false;
  }
}

void ReferenceProperty
::update_references(const std::map<std::size_t, AbstractPropertyOwner *> &references)
{
  if (m_reference_value_id != 0) {
    try {
      set(references.at(m_reference_value_id));
    } catch (const std::out_of_range&) {
      LWARNING << "Failed to restore reference for property " << label();
    }
  }
}

Property::Filter ReferenceProperty::filter() const
{
  return configuration.get<Property::Filter>(ReferenceProperty::FILTER_POINTER);
}

void ReferenceProperty::revise() { set(nullptr); }

void ReferenceProperty::set(AbstractPropertyOwner * const &value)
{
  AbstractPropertyOwner* const old_value = this->value();
  TypedProperty::set(value);
  Q_EMIT reference_changed(old_value, value);
}

ReferenceProperty::Filter::Filter(const Disjunction<Kind>& kind, const DNF<Flag>& flag)
  : kind(kind), flag(flag)
{
}

Property::Filter::Filter()
{
}

ReferenceProperty::Filter::Filter(const DNF<Flag>& flag)
  : Filter(Disjunction<Kind>(Kind::All, Kind::None), flag)
{
}

}   // namespace omm
