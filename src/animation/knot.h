#pragma once

#include "serializers/abstractserializer.h"

namespace omm
{

struct Knot
{
  enum class Side { Left, Right };
  Knot(AbstractDeserializer& deserializer, const Serializable::Pointer& pointer, const QString& type);
  Knot(const variant_type& value);
  Knot(Knot&&) = delete;
  Knot& operator=(Knot&&) = delete;
  Knot& operator=(const Knot&) = delete;
  ~Knot() = default;
  void swap(Knot& other);
  [[nodiscard]] std::unique_ptr<Knot> clone() const;

  bool operator==(const Knot& other) const;
  bool operator!=(const Knot& other) const;

  variant_type value;
  variant_type& offset(Side side);
  variant_type left_offset;
  variant_type right_offset;

private:
  // copying the Knot is dangerous because the Deserializaer
  // may hold a pointer to this (as ReferencePolisher).
  // Use Knot::clone explicitely, if you know what you do.
  Knot(const Knot& other) = default;
  void polish();
  class ReferencePolisher;
};

}  // namespace omm
