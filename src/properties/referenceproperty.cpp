#include "properties/referenceproperty.h"
#include "objects/object.h"
#include "tags/tag.h"

namespace omm
{

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
  , m_referenceproperty_reference_observer(*this)
{
  set_default_value(nullptr);
}

ReferenceProperty::ReferenceProperty(const ReferenceProperty &other)
  : TypedProperty<AbstractPropertyOwner *>(other)
  , m_referenceproperty_reference_observer(*this)
{
  value()->register_observer(m_referenceproperty_reference_observer);
}

ReferenceProperty::~ReferenceProperty()
{
  NotificationBlocker blocker(*this);
  set(nullptr);
}

std::string ReferenceProperty::type() const { return TYPE; }

void ReferenceProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  // TODO serialize allowed_kinds and required_flags
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
}

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  // TODO deserialize allowed_kinds and required_flags
  TypedProperty::deserialize(deserializer, root);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto id = deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  deserializer.register_reference_property(*this, id);
}

void ReferenceProperty::set_default_value(const value_type& value)
{
  assert(value == nullptr);
  TypedProperty::set_default_value(value);
}

ReferenceProperty& ReferenceProperty::set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds)
{
  m_allowed_kinds = allowed_kinds;
  return *this;
}

ReferenceProperty&
ReferenceProperty::set_required_flags(AbstractPropertyOwner::Flag required_flags)
{
  m_required_flags = required_flags;
  return *this;
}

AbstractPropertyOwner::Kind ReferenceProperty::allowed_kinds() const { return m_allowed_kinds; }
AbstractPropertyOwner::Flag ReferenceProperty::required_flags() const { return m_required_flags; }

bool ReferenceProperty::is_compatible(const Property& other) const
{
  if (Property::is_compatible(other)) {
    auto& other_reference_property = static_cast<const ReferenceProperty&>(other);
    return other_reference_property.allowed_kinds() == allowed_kinds();
  } else {
    return false;
  }
}

std::unique_ptr<Property> ReferenceProperty::clone() const
{
  return std::make_unique<ReferenceProperty>(*this);
}

void ReferenceProperty::set(AbstractPropertyOwner * const &apo)
{
  auto* old_apo = value();
  if (old_apo) {
    old_apo->unregister_observer(m_referenceproperty_reference_observer);
  }
  TypedProperty::set(apo);
  if (apo) {
    apo->register_observer(m_referenceproperty_reference_observer);
  }
}

void ReferenceProperty::revise() { set(nullptr); }

ReferencePropertyReferenceObserver
::ReferencePropertyReferenceObserver(ReferenceProperty &master_property)
  : m_master_property(master_property) {}

void ReferencePropertyReferenceObserver::on_change(AbstractPropertyOwner *, int, Property *, std::set<const void *> trace)
{
  if (::contains(trace, this)) {
    LINFO << "cycle!";
  } else {
    trace.insert(this);
    m_master_property.notify_observers(trace);
  }
}

}   // namespace omm
