#pragma once

#include <unordered_set>

namespace omm
{

template<typename ValueT>
class MultiValueEdit
{
public:
  using value_type = ValueT;
  using Values = std::unordered_set<value_type>;

  virtual void set_values(const Values& values)
  {
    assert(values.size() > 0);
    const value_type& the_value = *std::begin(values);
    for (const value_type& current_value : values) {
      if (current_value != the_value) {
        set_inconsistent_value();
        return;
      }
    }
    set_value(the_value);
  }

  virtual void set_value(const value_type& value) = 0;
  virtual void set_inconsistent_value() = 0;
  virtual value_type value() const = 0;
};

}  // namespace omm
