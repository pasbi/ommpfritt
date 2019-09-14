#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class NumericPropertyDetail
{
public:
  static constexpr auto LOWER_VALUE_POINTER = QT_TRANSLATE_NOOP("NumericProperty", "lower_value");
  static constexpr auto UPPER_VALUE_POINTER = QT_TRANSLATE_NOOP("NumericProperty", "upper_value");
  static constexpr auto STEP_POINTER = QT_TRANSLATE_NOOP("NumericProperty", "step");
  static constexpr auto MULTIPLIER_POINTER = QT_TRANSLATE_NOOP("NumericProperty", "multiplier");
};

template<typename T> class NumericProperty : public TypedProperty<T>
{
public:
  using D = NumericPropertyDetail;
  using value_type = T;

  explicit NumericProperty(const T& default_value = T()) : TypedProperty<T>(default_value)
  {
    this->m_configuration[D::MULTIPLIER_POINTER] = 1.0;
    this->m_configuration[D::STEP_POINTER] = default_step;
    this->m_configuration[D::LOWER_VALUE_POINTER] = lowest_possible_value;
    this->m_configuration[D::UPPER_VALUE_POINTER] = highest_possible_value;
  }

  void set(const Property::variant_type& variant) override
  {
    if (std::holds_alternative<int>(variant)) {
      TypedProperty<T>::set(std::get<int>(variant));
    } else {
      TypedProperty<T>::set(std::get<T>(variant));
    }
  }

  NumericProperty<T>& set_range(const T& lower, const T& upper)
  {
    this->m_configuration[D::LOWER_VALUE_POINTER] = lower;
    this->m_configuration[D::UPPER_VALUE_POINTER] = upper;
    return *this;
  }

  NumericProperty<T>& set_step(const T& step)
  {
    this->m_configuration[D::STEP_POINTER] = step;
    return *this;
  }

  NumericProperty<T>& set_multiplier(double multiplier)
  {
    this->m_configuration[D::MULTIPLIER_POINTER] = multiplier;
    return *this;
  }

  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root) override
  {
    TypedProperty<T>::deserialize(deserializer, root);
    for (const std::string& key : { D::LOWER_VALUE_POINTER, D::UPPER_VALUE_POINTER, D::STEP_POINTER }) {
      this->m_configuration[key] = deserializer.get<T>(Serializable::make_pointer(root, key));
    }
    this->m_configuration[D::MULTIPLIER_POINTER]
        = deserializer.get<double>(Serializable::make_pointer(root, D::MULTIPLIER_POINTER));
  }

  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const override
  {
    TypedProperty<T>::serialize(serializer, root);
    for (const std::string& key : { D::LOWER_VALUE_POINTER, D::UPPER_VALUE_POINTER, D::STEP_POINTER }) {
      serializer.set_value(this->m_configuration.template get<T>(key),
                                 Serializable::make_pointer(root, key));
    }
    serializer.set_value(this->m_configuration.template get<double>(D::MULTIPLIER_POINTER),
                         Serializable::make_pointer(root, D::MULTIPLIER_POINTER));
  }

  void revise() override
  {
    set(std::clamp(this->value(),
                   this->m_configuration.template get<T>(D::LOWER_VALUE_POINTER),
                   this->m_configuration.template get<T>(D::UPPER_VALUE_POINTER)));
  }

  static const T highest_possible_value;
  static const T lowest_possible_value;
  static const T smallest_step;
  static const T default_step;

};


}  // namespace omm
