#include "animation/knot.h"

namespace omm
{
Knot::Knot(AbstractDeserializer& deserializer, const Serializable::Pointer& pointer, const QString& type)
{
  if (type == "Reference") {
    m_reference_id = deserializer.get<std::size_t>(pointer);
    deserializer.register_reference_polisher(*this);
    value = nullptr;
  } else {
    value = deserializer.get(pointer, type);
    polish();
  }
}

Knot::Knot(const variant_type& value) : value(value)
{
  polish();
}

void Knot::swap(Knot& other)
{
  std::swap(other.value, value);
  std::swap(other.left_offset, left_offset);
  std::swap(other.right_offset, right_offset);
  std::swap(other.m_reference_id, m_reference_id);
}

std::unique_ptr<Knot> Knot::clone() const
{
  return std::unique_ptr<Knot>(new Knot(*this));
}

void Knot::update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map)
{
  if (m_reference_id == 0) {
    value = nullptr;
  } else {
    value = map.at(m_reference_id);
  }
}

bool Knot::operator==(const Knot& other) const
{
  return value == other.value && left_offset == other.left_offset
         && right_offset == other.right_offset;
}

bool Knot::operator!=(const Knot& other) const
{
  return !(*this == other);
}

variant_type& Knot::offset(Side side)
{
  switch (side) {
  case Side::Left:
    return left_offset;
  case Side::Right:
    return right_offset;
  default:
    Q_UNREACHABLE();
    return right_offset;
  }
}

void Knot::polish()
{
  std::visit(
      [this](auto&& vv) {
        using T = std::decay_t<decltype(vv)>;
        if constexpr (n_channels<T>() > 0) {
          left_offset = null_value<T>();
          right_offset = null_value<T>();
        } else {
          // don't care about non numeric types.
        }
      },
      value);
}
}  // namespace omm
