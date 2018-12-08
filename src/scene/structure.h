#pragma once

#include <glog/logging.h>
#include "observed.h"

namespace omm
{

template<typename T> class Structure
{
public:
  using item_type = T;
  Structure() {}
  virtual ~Structure() {}

  virtual std::set<T*> items() const = 0;
  virtual size_t position(const T& item) const = 0;
  virtual const T* predecessor(const T& sibling) const = 0;

  std::set<T*> selected_items() const
  {
    const auto is_selected = [](const auto* t) { return t->is_selected(); };
    return ::filter_if(items(), is_selected);
  }

private:
  // we don't want to assign copy or move
  const Structure<T>& operator=(const Structure<T>&) = delete;
  const Structure<T>& operator=(Structure<T>&&) = delete;
  Structure<T>(const Structure<T>&) = delete;
  Structure<T>(Structure<T>&&) = delete;
};

}  // namespace omm
