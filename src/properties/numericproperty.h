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

  static constexpr auto MIN_VALUE_POINTER = "min_value";
  static constexpr auto MAX_VALUE_POINTER = "max_value";
  static constexpr auto STEP_POINTER = "step";
  static constexpr auto MULTIPLIER_POINTER = "multiplier";

  NumericProperty<T>& set_range(T min, T max)
  {
    m_min = min;
    m_max = max;
    return *this;
  }

  NumericProperty<T>& set_step(T step)
  {
    m_step = step;
    return *this;
  }

  NumericProperty<T>& set_multiplier(double multiplier)
  {
    m_multiplier = multiplier;
    return *this;
  }


  static constexpr T upper_limit()
  {
    if constexpr(std::numeric_limits<T>::has_infinity) {
      return std::numeric_limits<T>::infinity();
    } else {
      return std::numeric_limits<T>::max();
    }
  }

  static constexpr T lower_limit()
  {
    if constexpr(std::numeric_limits<T>::has_infinity) {
      return -std::numeric_limits<T>::infinity();
    } else {
      return std::numeric_limits<T>::lowest();
    }
  }

  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
  {
    m_min = deserializer.get_double(Serializable::make_pointer(root, MIN_VALUE_POINTER));
    m_max = deserializer.get_double(Serializable::make_pointer(root, MAX_VALUE_POINTER));
    m_step = deserializer.get_double(Serializable::make_pointer(root, STEP_POINTER));
    m_multiplier = deserializer.get_double(Serializable::make_pointer(root, MULTIPLIER_POINTER));

  }

  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
  {
    serializer.set_value(m_min, Serializable::make_pointer(root, MIN_VALUE_POINTER));
    serializer.set_value(m_max, Serializable::make_pointer(root, MAX_VALUE_POINTER));
    serializer.set_value(m_step, Serializable::make_pointer(root, STEP_POINTER));
    serializer.set_value(m_multiplier, Serializable::make_pointer(root, MULTIPLIER_POINTER));
  }

private:
  T m_min = lower_limit();
  T m_max = upper_limit();
  T m_step = 1.0;
  double m_multiplier = 1.0;

  // these values are merely reommendations for the gui.
  T min() const { return m_min; }
  T max() const { return m_max; }
  T step() const { return m_step; }
  double multiplier() const { return m_multiplier; }
};

}  // namespace omm
