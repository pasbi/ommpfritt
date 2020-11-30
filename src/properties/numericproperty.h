#pragma once

#include "properties/typedproperty.h"

namespace NumericPropertyDetail
{
static constexpr int int_high = std::numeric_limits<int>::max();
static constexpr int int_low = std::numeric_limits<int>::lowest();
static constexpr double inf = std::numeric_limits<double>::infinity();
}  // namespace NumericPropertyDetail

namespace omm
{
class NumericPropertyDetail
{
public:
  static constexpr auto LOWER_VALUE_POINTER = "lower_value";
  static constexpr auto UPPER_VALUE_POINTER = "upper_value";
  static constexpr auto STEP_POINTER = "step";
  static constexpr auto MULTIPLIER_POINTER = "multiplier";
  static constexpr auto PREFIX_POINTER = "prefix";
  static constexpr auto SUFFIX_POINTER = "suffix";
};

template<typename T> class NumericProperty : public TypedProperty<T>
{
public:
  using D = NumericPropertyDetail;
  using value_type = T;

  explicit NumericProperty(const T& default_value = T()) : TypedProperty<T>(default_value)
  {
    this->configuration.set(D::MULTIPLIER_POINTER, 1.0);
    this->configuration.set(D::STEP_POINTER, default_step());
    this->configuration.set(D::LOWER_VALUE_POINTER, lowest_possible_value());
    this->configuration.set(D::UPPER_VALUE_POINTER, highest_possible_value());
    this->configuration.set(D::PREFIX_POINTER, QString());
    this->configuration.set(D::SUFFIX_POINTER, QString());
  }

  void set(const variant_type& variant) override
  {
    if (std::holds_alternative<int>(variant)) {
      TypedProperty<T>::set(std::get<int>(variant));
    } else {
      TypedProperty<T>::set(std::get<T>(variant));
    }
  }

  NumericProperty<T>& set_range(const T& lower, const T& upper)
  {
    this->configuration.set(D::LOWER_VALUE_POINTER, lower);
    this->configuration.set(D::UPPER_VALUE_POINTER, upper);
    Q_EMIT this->configuration_changed();
    return *this;
  }

  NumericProperty<T>& set_prefix(const QString& prefix)
  {
    this->configuration.at(D::PREFIX_POINTER) = prefix;
    Q_EMIT this->configuration_changed();
    return *this;
  }

  NumericProperty<T>& set_suffix(const QString& suffix)
  {
    this->configuration.set(D::SUFFIX_POINTER, suffix);
    Q_EMIT this->configuration_changed();
    return *this;
  }

  NumericProperty<T>& set_step(const T& step)
  {
    this->configuration.set(D::STEP_POINTER, step);
    Q_EMIT this->configuration_changed();
    return *this;
  }

  NumericProperty<T>& set_multiplier(double multiplier)
  {
    this->configuration.set(D::MULTIPLIER_POINTER, multiplier);
    Q_EMIT this->configuration_changed();
    return *this;
  }

  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root) override
  {
    TypedProperty<T>::deserialize(deserializer, root);
    if (this->is_user_property()) {
      for (const QString& key : {D::LOWER_VALUE_POINTER, D::UPPER_VALUE_POINTER, D::STEP_POINTER}) {
        this->configuration.set(key, deserializer.get<T>(Serializable::make_pointer(root, key)));
      }
      this->configuration.set(
          D::MULTIPLIER_POINTER,
          deserializer.get<double>(Serializable::make_pointer(root, D::MULTIPLIER_POINTER)));
    }
  }

  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const override
  {
    TypedProperty<T>::serialize(serializer, root);
    if (this->is_user_property()) {
      for (const QString& key : {D::LOWER_VALUE_POINTER, D::UPPER_VALUE_POINTER, D::STEP_POINTER}) {
        serializer.set_value(this->configuration.template get<T>(key),
                             Serializable::make_pointer(root, key));
      }
      serializer.set_value(this->configuration.template get<double>(D::MULTIPLIER_POINTER),
                           Serializable::make_pointer(root, D::MULTIPLIER_POINTER));
    }
  }

  void revise() override
  {
    set(std::clamp(this->value(),
                   this->configuration.template get<T>(D::LOWER_VALUE_POINTER),
                   this->configuration.template get<T>(D::UPPER_VALUE_POINTER)));
  }

  static T highest_possible_value();
  static T lowest_possible_value();
  static T smallest_step();
  static T default_step();
};

}  // namespace omm
