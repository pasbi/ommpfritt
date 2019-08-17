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
#include "scene/history/historymodel.h"
#include "tools/toolbox.h"
#include "scene/abstractstructureobserver.h"
#include "scene/pointselection.h"

namespace omm
{

class Command;
class Project;
class PythonEngine;

template<typename T> struct SceneStructure;
template<> struct SceneStructure<Object> { using type = Tree<Object>; };
template<> struct SceneStructure<Style> { using type = List<Style>; };

class Scene : public QObject
{
  Q_OBJECT
public:
  Scene(PythonEngine& python_engine);
  ~Scene();

  static constexpr auto TYPE = "Scene";

  Tree<Object> object_tree;
  ObjectTreeAdapter object_tree_adapter;

  List<Style> styles;
  ListAdapter<Style> style_list_adapter;

  template<typename ItemT> typename SceneStructure<ItemT>::type& structure();
  template<typename ItemT> const typename SceneStructure<ItemT>::type& structure() const;

  // ObjectView root_view();

public Q_SLOTS:
  void reset();

public:
  PythonEngine& python_engine;

  // === Tags  ======
public:
  std::set<Tag*> tags() const;
private:
  mutable bool m_tags_cache_is_dirty = true;
  mutable std::set<Tag*> m_tags_cache;

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  std::set<AbstractPropertyOwner*> selection() const;
  template<typename ItemT> std::set<ItemT*> item_selection() const
  {
    return kind_cast<ItemT>(m_item_selection.at(ItemT::KIND));
  }

private:
  std::map<AbstractPropertyOwner::Kind, std::set<AbstractPropertyOwner*>> m_item_selection;
  std::set<AbstractPropertyOwner*> m_selection;
  std::unique_ptr<Object> make_root();

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
  HistoryModel history;
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }
  void submit(std::unique_ptr<Command> command);

private:
  Scene(const Scene& other) = delete;
  Scene(Scene&& other) = delete;

public:
  ToolBox tool_box;
  void evaluate_tags();

public:
  bool can_remove( QWidget* parent, std::set<AbstractPropertyOwner*> selection,
                   std::set<Property*>& properties ) const;
  bool remove(QWidget* parent, const std::set<AbstractPropertyOwner*>& selection);

public:
  void update();
  void update_tool();
  bool contains(const AbstractPropertyOwner* apo) const;

public:
  PointSelection point_selection;

Q_SIGNALS:
  void repaint();
  void object_selection_changed(const std::set<Object*>& selection);
  void style_selection_changed(const std::set<Style*>& selection);
  void tag_selection_changed(const std::set<Tag*>& selection);
  void tool_selection_changed(const std::set<Tool*>& selection);

  /**
   * @brief selection_changed emitted when one or more items becomes selected or deselected
   * @param selection the items which are selected after the selection change.
   */
  void selection_changed(const std::set<AbstractPropertyOwner*>& selection);

  /**
   * @brief selection_changed emitted when one or more items of a certain kind becomes selected or
   *  deselected. This signal unifies the following four signals:
   * @see object_selection_changed
   * @see style_selection_changed
   * @see tag_selection_changed
   * @see tool_selection_changed
   * @param selection all items of a certain kind that are selected after the selection has changed.
   * @param kind the kind of the items.
   */
  void selection_changed(const std::set<AbstractPropertyOwner*>& selection,
                            AbstractPropertyOwner::Kind kind);
  void structure_changed();
  void scene_changed(AbstractPropertyOwner* subject, int code, Property* property);
  void filename_changed();

private:
  void prepare_reset();

};

}  // namespace omm

