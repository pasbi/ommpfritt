#pragma once

#include <set>
#include <memory>
#include "scene/abstractstructureobserver.h"
#include "observed.h"

namespace omm
{

class Scene;

class AbstractStructure : public QObject
{
  Q_OBJECT
Q_SIGNALS:
  void item_changed(std::set<const void*> trace);
  void structure_changed(std::set<const void*> trace);
};

template<typename T> class Structure : public AbstractStructure

{
public:
  using item_type = T;
  Structure() = default;
  Structure(Structure&& other) = default;
  virtual ~Structure() = default;

  virtual std::set<T*> items() const = 0;
  virtual size_t position(const T& item) const = 0;
  virtual const T* predecessor(const T& sibling) const = 0;
  T* predecessor(T& sibling) const;
  size_t insert_position(const T* predecessor) const;

  virtual std::unique_ptr<T> remove(T& t) = 0;

private:
  // we don't want to assign copy or move
  const Structure<T>& operator=(const Structure<T>&) = delete;
  const Structure<T>& operator=(Structure<T>&&) = delete;
  Structure<T>(const Structure<T>&) = delete;
};

}  // namespace omm
