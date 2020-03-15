#pragma once

#include "properties/typedproperty.h"
#include "aspects/propertyowner.h"
#include <Qt>
#include "dnf.h"

namespace omm
{

class AbstractPropertyOwner;
class ReferenceProperty;

class ReferenceProperty : public TypedProperty<AbstractPropertyOwner*>, ReferencePolisher
{
  Q_OBJECT
public:
  ReferenceProperty();
  explicit ReferenceProperty(const ReferenceProperty& other);
  ~ReferenceProperty() override;
  QString type() const override { return TYPE; }
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  static constexpr auto FILTER_POINTER = "filter";

  ReferenceProperty& set_filter(const Filter& filter);
  void revise() override;
  void set(AbstractPropertyOwner *const &value) override;

  bool is_compatible(const Property& other) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "ReferenceProperty");

  static std::vector<omm::AbstractPropertyOwner*>
  collect_candidates(const Scene& scene, const Kind kinds,
                                         const Flag flags);

  static const std::map<Kind, QString> KIND_KEYS;
  static const std::map<Flag, QString> FLAG_KEYS;
  void update_references(const std::map<std::size_t, AbstractPropertyOwner *> &references) override;

  static const PropertyDetail detail;
  Filter filter() const;

Q_SIGNALS:
  void reference_changed(AbstractPropertyOwner* old_ref, AbstractPropertyOwner* new_ref);

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;

  // this field is only required temporarily during deserialization
  std::size_t m_reference_value_id;
};

}  // namespace omm
