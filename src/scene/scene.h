#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdint>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "external/json_fwd.hpp"
#include "scene/objecttree.h"
#include "scene/contextes.h"
#include "scene/cachedgetter.h"
#include "scene/list.h"
#include "scene/history/historymodel.h"
#include "tools/toolbox.h"
#include "scene/pointselection.h"
#include "scene/stylelist.h"
#include "scene/cycleguard.h"
#include "scene/messagebox.h"

namespace omm
{

class Command;
class Project;
class PythonEngine;

template<typename T> struct SceneStructure;
template<> struct SceneStructure<Object> { using type = ObjectTree; };
template<> struct SceneStructure<Style> { using type = StyleList; };

class Scene : public QObject
{
  Q_OBJECT
public:
  Scene(PythonEngine& python_engine);
  ~Scene();

  static constexpr auto TYPE = "Scene";
  MessageBox message_box;
  ObjectTree object_tree;
  StyleList styles;

  template<typename ItemT> typename SceneStructure<ItemT>::type& structure();
  template<typename ItemT> const typename SceneStructure<ItemT>::type& structure() const;

public Q_SLOTS:
  void reset();

public:
  PythonEngine& python_engine;

  // === Tags  ======
public:
  std::set<Tag*> tags() const;

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
  bool contains(const AbstractPropertyOwner* apo) const;

public:
  PointSelection point_selection;

private:
  void prepare_reset();

public:
  [[nodiscard]] std::unique_ptr<CycleGuard> make_cycle_guard(const Object* guarded);
private:
  std::set<const Object*> m_cycle_guarded_objects;

public Q_SLOTS:
  void update_tool();

};

}  // namespace omm

