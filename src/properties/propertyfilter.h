#pragma once

#include "aspects/serializable.h"
#include "common.h"
#include "dnf.h"

namespace omm
{
struct PropertyFilter : public Serializable
{
  explicit PropertyFilter(const Disjunction<Kind>& kind, const DNF<Flag>& flag);
  PropertyFilter();
  explicit PropertyFilter(const DNF<Flag>& flag);
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  Disjunction<Kind> kind;
  DNF<Flag> flag;
  [[nodiscard]] bool accepts(const AbstractPropertyOwner& apo) const;
  [[nodiscard]] bool accepts(Kind kind, Flag flag) const;
  bool operator==(const PropertyFilter& other) const;
  bool operator!=(const PropertyFilter& other) const
  {
    return !(*this == other);
  }
  bool operator<(const PropertyFilter& other) const;

  static PropertyFilter accept_anything();
  QString to_string() const;
};

}  // namespace omm
