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
#include "scene/objecttreecontext.h"

namespace omm
{

class Scene;

class AbstractObjectTreeObserver
{
protected:
  virtual void beginInsertObjects(Object& parent, int start, int end) = 0;
  virtual void endInsertObjects() = 0;
  virtual void beginMoveObject(const ObjectTreeContext& new_context) = 0;
  virtual void endMoveObject() = 0;
  friend class Scene;
};

class AbstractPropertyObserver
{
protected:
  virtual void set_selection(const std::unordered_set<HasProperties*>& selection) = 0;
  friend class Scene;
};

class Object;
class Project;

class Scene
  : public ObserverRegister<AbstractObjectTreeObserver>
  , public ObserverRegister<AbstractPropertyObserver>
{
public:
  Scene(Project& project);
  ~Scene();

  ObjectView root_view();

  static Scene* currentInstance();
  Object& root() const;

  void reset();
  bool load(const nlohmann::json& data);
  nlohmann::json save() const;

  void insert_object(std::unique_ptr<Object> object, Object& parent);
  void move_object(const ObjectTreeContext& new_context);
  bool can_move_object(const ObjectTreeContext& new_context) const;
  void selection_changed();
  Project& project();


private:
  std::unique_ptr<Object> m_root;
  Project& m_project;
  static Scene* m_current;
};

}  // namespace omm

