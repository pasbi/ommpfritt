#pragma once

#include <map>
#include <memory>
#include <vector>
#include <unordered_set>
#include <stdint.h>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observerregister.h"
#include "scene/objecttreecontext.h"

namespace omm
{

class Scene;
class Command;

class AbstractObjectTreeObserver
{
protected:
  virtual void beginInsertObject(Object& parent, int row) = 0;
  virtual void beginInsertObject(const OwningObjectTreeContext& context) = 0;
  virtual void endInsertObject() = 0;
  virtual void beginMoveObject(const MoveObjectTreeContext& new_context) = 0;
  virtual void endMoveObject() = 0;
  virtual void beginRemoveObject(const Object& object) = 0;
  virtual void endRemoveObject() = 0;
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
  Scene();
  ~Scene();

  ObjectView root_view();

  static Scene* currentInstance();
  Object& root() const;

  /**
   * @return returns the old root
   */
  std::unique_ptr<Object> replace_root(std::unique_ptr<Object> new_root);

  void reset();

  void insert_object(std::unique_ptr<Object> object, Object& parent);
  void insert_object(OwningObjectTreeContext& context);
  void move_object(MoveObjectTreeContext context);
  void remove_object(OwningObjectTreeContext& context);
  bool can_move_object(const MoveObjectTreeContext& new_context) const;
  void selection_changed();

  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);

  std::string filename() const;
  template<typename CommandT, typename... Args> void submit(Args... args)
  {
    submit(std::make_unique<CommandT>(*this, std::forward<Args>(args)...));
  }

  bool has_pending_changes() const;
  QUndoStack& undo_stack();

private:
  void submit(std::unique_ptr<Command> command);
  void set_has_pending_changes(bool v);
  std::unique_ptr<Object> m_root;
  static Scene* m_current;

  /**
   * holds the last filename this scene was associated to.
   * is set in `save_as` and `load_from`
   */
  std::string m_filename;
  bool m_has_pending_changes = false;
  QUndoStack m_undo_stack;
};

}  // namespace omm

