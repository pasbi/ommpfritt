#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdint>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "external/json_fwd.hpp"
#include "observed.h"
#include "scene/contextes.h"
#include "scene/cachedgetter.h"
#include "scene/list.h"
#include "scene/tree.h"
#include "scene/listadapter.h"
#include "scene/objecttreeadapter.h"
#include "scene/abstractselectionobserver.h"
#include "tools/toolbox.h"

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
  Scene(PythonEngine& python_engine);
  ~Scene();

  std::unique_ptr<Object> make_root();
  static constexpr auto TYPE = "Scene";

  Tree<Object> object_tree;
  ObjectTreeAdapter object_tree_adapter;

  List<Style> styles;
  ListAdapter<Style> style_list_adapter;

  template<typename ItemT> typename SceneStructure<ItemT>::type& structure();
  template<typename ItemT> const typename SceneStructure<ItemT>::type& structure() const;

  // ObjectView root_view();
  void reset();
  PythonEngine& python_engine;

  // === Tags  ======
public:
  std::set<Tag*> tags() const;
private:
  mutable bool m_tags_cache_is_dirty = true;
  mutable std::set<Tag*> m_tags_cache;

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  std::set<Object*> object_selection() const;

private:
  std::set<Object*> m_object_selection;

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

  template<typename T> std::set<T*> find_items(const std::string& name) const;

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
  QUndoStack undo_stack;
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }
  bool has_pending_changes() const;
  void submit(std::unique_ptr<Command> command);

private:
  bool m_has_pending_changes = false;
  void set_has_pending_changes(bool v);

public:
  static Scene* currentInstance();
private:
  static Scene* m_current;

private:
  Scene(const Scene& other) = delete;
  Scene(Scene&& other) = delete;

public:
  ToolBox tool_box;

  template<typename ObjectT, typename F> void for_each_selected_object(F&& f)
  {
    for (auto&& o : object_selection()) {
      if (o->type() == ObjectT::TYPE) {
        f(static_cast<ObjectT*>(o));
      }
    }
  }

  void evaluate_tags();

};

}  // namespace omm

