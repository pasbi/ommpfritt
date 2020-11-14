#pragma once

#include <cassert>
#include <functional>
#include <set>

namespace omm
{
template<typename ValueT, typename EqualT = std::equal_to<ValueT>> class MultiValueEdit
{
public:
  using value_type = ValueT;
  using Values = std::set<ValueT>;

  explicit MultiValueEdit() = default;

  virtual void set_values(const Values& values)
  {
    assert(values.size() > 0);
    const value_type& the_value = *std::begin(values);
    for (const value_type& current_value : values) {
      if (!EqualT{}(current_value, the_value)) {
        set_inconsistent_value();
        return;
      }
    }
    set_value(the_value);
  }

  virtual void set_value(const value_type& value) = 0;
  virtual value_type value() const = 0;

protected:
  virtual void set_inconsistent_value() = 0;
};

}  // namespace omm
