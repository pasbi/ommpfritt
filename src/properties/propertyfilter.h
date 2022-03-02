#pragma once

#include "common.h"
#include "dnf.h"

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

struct PropertyFilter
{
  explicit PropertyFilter(const Disjunction<Kind>& kind, const DNF<Flag>& flag);
  PropertyFilter();
  explicit PropertyFilter(const DNF<Flag>& flag);
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
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
