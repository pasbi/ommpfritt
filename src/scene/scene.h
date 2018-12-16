#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdint>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observed.h"
#include "scene/contextes.h"
#include "scene/cachedgetter.h"
#include "scene/list.h"
#include "scene/tree.h"
#include "scene/stylelistadapter.h"
#include "scene/objecttreeadapter.h"
#include "scene/abstractselectionobserver.h"

namespace omm
{

class Command;
class Project;
class PythonEngine;

template<typename T> struct SceneStructure;
template<> struct SceneStructure<Object> { using type = Tree<Object>; };
template<> struct SceneStructure<Style> { using type = List<Style>; };

class Scene
  : public Observed<AbstractSelectionObserver>
  , public Observed<AbstractSimpleStructureObserver>
{
public:
  Scene(const PythonEngine& python_engine);
  ~Scene();

  Tree<Object> object_tree;
  ObjectTreeAdapter object_tree_adapter;

  List<Style> styles;
  StyleListAdapter style_list_adapter;

  template<typename ItemT> typename SceneStructure<ItemT>::type& structure();
  template<typename ItemT> const typename SceneStructure<ItemT>::type& structure() const;

  ObjectView root_view();
  void reset();
  const PythonEngine& python_engine;

  // === Tags  ======
public:
  std::set<Tag*> tags() const;
private:
  mutable bool m_tags_cache_is_dirty = true;
  mutable std::set<Tag*> m_tags_cache;

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);

  // === Styles  ====
public:
  Style& default_style() const;
private:
  std::unique_ptr<Style> m_default_style;

  // === Objects, Tags and Styles ===
public:
  std::set<AbstractPropertyOwner*> property_owners() const;

  std::set<ReferenceProperty*>
  find_reference_holders(const AbstractPropertyOwner& candidate) const;

  std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>>
  find_reference_holders(const std::set<AbstractPropertyOwner*>& candidates) const;

  bool is_referenced(const AbstractPropertyOwner& candidate) const;
  void invalidate();

  // === Save/Load ====
public:
  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);
  std::string filename() const;
private:
  /**
   * holds the last filename this scene was associated to.
   * is set in `save_as` and `load_from`
   */
  std::string m_filename;

  // === Commands ====
public:
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }
  bool has_pending_changes() const;
  QUndoStack& undo_stack();
private:
  void submit(std::unique_ptr<Command> command);
  bool m_has_pending_changes = false;
  void set_has_pending_changes(bool v);
  QUndoStack m_undo_stack;



public:
  static Scene* currentInstance();
private:
  static Scene* m_current;

private:
  Scene(const Scene& other) = delete;
  Scene(Scene&& other) = delete;

};

}  // namespace omm

