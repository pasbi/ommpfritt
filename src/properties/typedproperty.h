#pragma once

#include "properties/property.h"

namespace omm
{

template<typename ValueT>
class TypedProperty : public Property, public ObserverRegister<AbstractTypedPropertyObserver>
{
public:
  using value_type = ValueT;

  TypedProperty(ValueT defaultValue = ValueT())
    : m_value(defaultValue), m_defaultValue(defaultValue) {}

public:
  virtual void set_value(ValueT value) { m_value = value; }
  virtual ValueT value() const { return m_value; }
  virtual ValueT& value() { return m_value; }
  virtual void reset() {  m_value = m_defaultValue; }
  py::object get_py_object() const override { return py::object(); }
  void set_py_object(const py::object& value) override {}

  std::string type() const override = 0;

private:
  ValueT m_value;
  ValueT m_defaultValue;
};

template<typename ValueT> std::string TypedProperty<ValueT>::type() const
{
  // TypedProperty<ValueT> has no meaningful way to provide type(), which is realised
  // by overriding the virtual type() member in any of the derived types.
  LOG(FATAL) << "It is tempting to call me. However, I'm useless. Don't call me.";
  return "";
}

}  // namespace
