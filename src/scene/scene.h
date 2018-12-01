#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <stdint.h>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observed.h"
#include "scene/objecttreecontext.h"
#include "scene/abstractselectionobserver.h"
#include "scene/abstractobjecttreeobserver.h"
#include "scene/stylepool.h"

namespace omm
{

class Command;
class Object;
class Project;

class Scene
  : public Observed<AbstractObjectTreeObserver>
  , public Observed<AbstractSelectionObserver>
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

  std::set<Object*> objects() const;
  std::set<Object*> selected_objects() const;
  std::set<Tag*> tags() const;
  std::set<Tag*> selected_tags() const;
  std::set<AbstractPropertyOwner*> selection() const;
  std::set<AbstractPropertyOwner*> property_owners() const;

  bool is_referenced(const AbstractPropertyOwner& candidate) const;

  void clear_selection();
  void selection_changed();

  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);

  std::string filename() const;
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }

  bool has_pending_changes() const;
  QUndoStack& undo_stack();
  StylePool& style_pool();
  const StylePool& style_pool() const;

private:
  void submit(std::unique_ptr<Command> command);
  void set_has_pending_changes(bool v);
  std::unique_ptr<Object> m_root;
  StylePool m_style_pool;
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

