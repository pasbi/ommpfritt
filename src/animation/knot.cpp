#include "animation/knot.h"
#include "serializers/deserializerworker.h"
#include "serializers/abstractdeserializer.h"

namespace omm
{

class Knot::ReferencePolisher : public omm::serialization::ReferencePolisher
{
public:
  explicit ReferencePolisher(std::size_t reference_id, Knot& knot)
    : m_reference_id(reference_id)
    , m_knot(knot)
  {
  }

private:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override
  {
    if (m_reference_id == 0) {
      m_knot.value = nullptr;
    } else {
      m_knot.value = map.at(m_reference_id);
    }
  }

  const std::size_t m_reference_id;
  Knot& m_knot;
};

Knot::Knot(serialization::DeserializerWorker& worker, const QString& type)
{
  if (type == "Reference") {
    const auto reference_id = worker.get<std::size_t>();
    auto ref_polisher = std::make_unique<ReferencePolisher>(reference_id, *this);
    worker.deserializer().register_reference_polisher(std::move(ref_polisher));
    value = nullptr;
  } else {
    value = worker.get(type);
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
}

std::unique_ptr<Knot> Knot::clone() const
{
  return std::unique_ptr<Knot>(new Knot(*this));
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
