#pragma once

#include "properties/typedproperty.h"
#include "aspects/propertyowner.h"
#include <Qt>

namespace omm
{

class AbstractPropertyOwner;
class ReferenceProperty;

class ReferencePropertyReferenceObserver : public AbstractPropertyOwnerObserver
{
public:
  ReferencePropertyReferenceObserver(ReferenceProperty& master_property);
  void on_change(AbstractPropertyOwner*) override;

private:
  ReferenceProperty& m_master_property;
};

class ReferenceProperty : public TypedProperty<AbstractPropertyOwner*>
{
public:
  ReferenceProperty();
  std::string type() const override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  ReferenceProperty& set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds);
  AbstractPropertyOwner::Kind allowed_kinds() const;
  ReferenceProperty& set_required_flags(AbstractPropertyOwner::Flag required_flags);
  AbstractPropertyOwner::Flag required_flags() const;
  void revise() override;

  bool is_compatible(const Property& other) const override;
  bool is_cyclic() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("ReferenceProperty", "ReferenceProperty");
  std::unique_ptr<Property> clone() const override;


  static std::vector<omm::AbstractPropertyOwner*>
  collect_candidates(const Scene& scene, const AbstractPropertyOwner::Kind allowed_kinds,
                                         const AbstractPropertyOwner::Flag required_flags);

  void set(AbstractPropertyOwner* const& apo) override;

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;
  AbstractPropertyOwner::Kind m_allowed_kinds = AbstractPropertyOwner::Kind::All;
  AbstractPropertyOwner::Flag m_required_flags = AbstractPropertyOwner::Flag::None;
  ReferencePropertyReferenceObserver m_referenceproperty_reference_observer;
};

}  // namespace omm
