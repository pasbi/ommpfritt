#pragma once

#include "aspects/propertyowner.h"
#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{
class AbstractPropertyOwner;
class ReferenceProperty;

class ReferenceProperty : public TypedProperty<AbstractPropertyOwner*>
{
  Q_OBJECT
public:
  ReferenceProperty();
  explicit ReferenceProperty(const ReferenceProperty& other);
  ~ReferenceProperty() override;

  ReferenceProperty(ReferenceProperty&&) = delete;
  ReferenceProperty& operator=(ReferenceProperty&&) = delete;
  ReferenceProperty& operator=(const ReferenceProperty&) = delete;

  void serialize(serialization::SerializerWorker& worker) const override;
  void deserialize(serialization::DeserializerWorker& worker) override;

  static constexpr auto FILTER_POINTER = "filter";

  ReferenceProperty& set_filter(const PropertyFilter& filter);
  void revise() override;
  void set(AbstractPropertyOwner* const& value) override;

  [[nodiscard]] bool is_compatible(const Property& other) const override;

  static std::vector<omm::AbstractPropertyOwner*>
  collect_candidates(const Scene& scene, Kind kinds, Flag flags);

  static const std::map<Kind, QString> KIND_KEYS;
  static const std::map<Flag, QString> FLAG_KEYS;

  static const PropertyDetail detail;
  [[nodiscard]] PropertyFilter filter() const;

Q_SIGNALS:
  void reference_changed(omm::AbstractPropertyOwner* old_ref, omm::AbstractPropertyOwner* new_ref);

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;
  class ReferencePolisher;
};

}  // namespace omm
