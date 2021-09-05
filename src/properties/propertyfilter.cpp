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

void PropertyFilter::deserialize(AbstractDeserializer& deserializer,
                                 const Serializable::Pointer& root)
{
  deserializer.get(kind, make_pointer(root, "kind"));
  deserializer.get(flag, make_pointer(root, "flag"));
}

bool PropertyFilter::accepts(Kind kind, Flag flag) const
{
  return this->flag.evaluate(flag) && this->kind.evaluate(kind);
}

void PropertyFilter::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  serializer.set_value(kind, make_pointer(root, "kind"));
  serializer.set_value(flag, make_pointer(root, "flag"));
}

bool PropertyFilter::accepts(const AbstractPropertyOwner& apo) const
{
  return accepts(apo.kind, apo.flags());
}

bool PropertyFilter::operator==(const PropertyFilter& other) const
{
  return kind == other.kind && flag == other.flag;
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
  return QString{"Filter[Flag=%1, Kind=%2]"}.arg(flag.to_string()).arg(kind.to_string());
}
}  // namespace omm
