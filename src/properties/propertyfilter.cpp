#include "properties/propertyfilter.h"
#include "aspects/abstractpropertyowner.h"

namespace omm
{
PropertyFilter::PropertyFilter(const Disjunction<Kind>& kind, const DNF<Flag>& flag)
    : kind(kind), flag(flag)
{
}

PropertyFilter::PropertyFilter() = default;

PropertyFilter::PropertyFilter(const DNF<Flag>& flag)
    : PropertyFilter(Disjunction<Kind>(Kind::All, Kind::None), flag)
{
}

void PropertyFilter::deserialize(serialization::DeserializerWorker& worker)
{
  kind.deserialize(*worker.sub("kind"));
  flag.deserialize(*worker.sub("flag"));
}

bool PropertyFilter::accepts(Kind kind, Flag flag) const
{
  return this->flag.evaluate(flag) && this->kind.evaluate(kind);
}

void PropertyFilter::serialize(serialization::SerializerWorker& worker) const
{
  kind.serialize(*worker.sub("kind"));
  flag.serialize(*worker.sub("flag"));
}

bool PropertyFilter::accepts(const AbstractPropertyOwner& apo) const
{
  return accepts(apo.kind, apo.flags());
}

bool PropertyFilter::operator==(const PropertyFilter& other) const
{
  return kind == other.kind && flag == other.flag;
}

bool PropertyFilter::operator!=(const PropertyFilter& other) const
{
  return !(*this == other);
}

bool PropertyFilter::operator<(const PropertyFilter& other) const
{
  if (kind == other.kind) {
    return flag < other.flag;
  } else {
    return kind == other.kind;
  }
}

PropertyFilter PropertyFilter::accept_anything()
{
  return PropertyFilter(Disjunction<Kind>(Kind::All, Kind::None), {{}});
}

QString PropertyFilter::to_string() const
{
  return QString{"Filter[Flag=%1, Kind=%2]"}.arg(flag.to_string(), kind.to_string());
}

}  // namespace omm
