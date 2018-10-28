#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include <QAbstractItemModel>
#include <unordered_set>

namespace omm
{

class Scene;

class AbstractObjectTreeAdapter
{
protected:
  virtual void beginInsertObjects(Object& parent, int start, int end) {}
  virtual void endInsertObjects() {}

  friend class Scene;
};

class Object;

class Scene
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

  void register_object_tree_adapter(AbstractObjectTreeAdapter& adapter);
  void unregister_object_tree_adapter(AbstractObjectTreeAdapter& adapter);

private:
  std::unique_ptr<Object> m_root;
  std::unordered_set<AbstractObjectTreeAdapter*> m_adapters;
  static Scene* m_current;
};

}  // namespace omm

