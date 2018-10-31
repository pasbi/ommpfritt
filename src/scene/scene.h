#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include <QAbstractItemModel>
#include <unordered_set>
#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observerregister.h"

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
protected:
  virtual void beginInsertObjects(Object& parent, int start, int end) = 0;
  virtual void endInsertObjects() = 0;
  friend class Scene;
};

class AbstractPropertyObserver
{
protected:
  virtual void set_selection(const std::unordered_set<HasProperties*>& selection) = 0;
  friend class Scene;
};

class Object;

class Scene
  : public ObserverRegister<AbstractObjectTreeObserver>
  , public ObserverRegister<AbstractPropertyObserver>

{
public:
  Scene();
  ~Scene();

  ObjectView root_view();

  static Scene* currentInstance();
  Object& root() const;

  void reset();
  bool load(const nlohmann::json& data);
  nlohmann::json save() const;

  void insert_object(std::unique_ptr<Object> object, Object& parent);
  void selection_changed();

private:
  std::unique_ptr<Object> m_root;
  static Scene* m_current;
};

}  // namespace omm

