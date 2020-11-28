#pragma once

#include "aspects/propertyowner.h"
#include "dnf.h"
#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{
class AbstractPropertyOwner;
class ReferenceProperty;

class ReferenceProperty
    : public TypedProperty<AbstractPropertyOwner*>
    , ReferencePolisher
{
  Q_OBJECT
public:
  ReferenceProperty();
  explicit ReferenceProperty(const ReferenceProperty& other);
  ~ReferenceProperty() override;

  ReferenceProperty(ReferenceProperty&&) = delete;
  ReferenceProperty& operator=(ReferenceProperty&&) = delete;
  ReferenceProperty& operator=(const ReferenceProperty&) = delete;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  static constexpr auto FILTER_POINTER = "filter";

  ReferenceProperty& set_filter(const Filter& filter);
  void revise() override;
  void set(AbstractPropertyOwner* const& value) override;

  [[nodiscard]] bool is_compatible(const Property& other) const override;

  static std::vector<omm::AbstractPropertyOwner*>
  collect_candidates(const Scene& scene, const Kind kinds, const Flag flags);

  static const std::map<Kind, QString> KIND_KEYS;
  static const std::map<Flag, QString> FLAG_KEYS;
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& references) override;

  static const PropertyDetail detail;
  [[nodiscard]] Filter filter() const;

Q_SIGNALS:
  void reference_changed(omm::AbstractPropertyOwner* old_ref, omm::AbstractPropertyOwner* new_ref);

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;

  // this field is only required temporarily during deserialization
  std::size_t m_reference_value_id{};
};

}  // namespace omm
