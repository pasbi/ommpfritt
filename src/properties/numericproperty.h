#pragma once

#include "properties/typedproperty.h"

namespace omm
{

template<typename T>
class NumericProperty : public TypedProperty<T>
{
public:
  using value_type = T;
  using TypedProperty<T>::TypedProperty;

  void set(const Property::variant_type& variant) override
  {
    if (std::holds_alternative<int>(variant)) {
      TypedProperty<T>::set(std::get<int>(variant));
    } else {
      TypedProperty<T>::set(std::get<T>(variant));
    }
  }

  static constexpr auto LOWER_VALUE_POINTER = "lower_value";
  static constexpr auto UPPER_VALUE_POINTER = "upper_value";
  static constexpr auto STEP_POINTER = "step";
  static constexpr auto MULTIPLIER_POINTER = "multiplier";

  NumericProperty<T>& set_range(const T& lower, const T& upper)
  {
    m_lower = lower;
    m_upper = upper;
    return *this;
  }

  NumericProperty<T>& set_step(const T& step)
  {
    m_step = step;
    return *this;
  }

  NumericProperty<T>& set_multiplier(double multiplier)
  {
    m_multiplier = multiplier;
    return *this;
  }

  static constexpr T get_upper_limit()
  {
    if constexpr(std::numeric_limits<T>::has_infinity) {
      return std::numeric_limits<T>::infinity();
    } else {
      return std::numeric_limits<T>::max();
    }
  }

  static constexpr T get_lower_limit()
  {
    if constexpr(std::numeric_limits<T>::has_infinity) {
      return -std::numeric_limits<T>::infinity();
    } else {
      return std::numeric_limits<T>::lowest();
    }
  }

  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
  {
    m_lower = deserializer.get_double(Serializable::make_pointer(root, LOWER_VALUE_POINTER));
    m_upper = deserializer.get_double(Serializable::make_pointer(root, UPPER_VALUE_POINTER));
    m_step = deserializer.get_double(Serializable::make_pointer(root, STEP_POINTER));
    m_multiplier = deserializer.get_double(Serializable::make_pointer(root, MULTIPLIER_POINTER));

  }

  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
  {
    serializer.set_value(m_lower, Serializable::make_pointer(root, LOWER_VALUE_POINTER));
    serializer.set_value(m_upper, Serializable::make_pointer(root, UPPER_VALUE_POINTER));
    serializer.set_value(m_step, Serializable::make_pointer(root, STEP_POINTER));
    serializer.set_value(m_multiplier, Serializable::make_pointer(root, MULTIPLIER_POINTER));
  }

private:
  T m_lower = get_lower_limit();
  T m_upper = get_upper_limit();
  T m_step = 1.0;
  double m_multiplier = 1.0;

public:
  // these values are merely reommendations for the gui.
  T lower() const { return m_lower; }
  T upper() const { return m_upper; }
  T step() const { return m_step; }
  double multiplier() const { return m_multiplier; }
};

}  // namespace omm
