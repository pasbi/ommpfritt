#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <cstdint>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "external/json_fwd.hpp"
#include "scene/cycleguard.h"
#include "scene/contextes.h"
#include "cachedgetter.h"
#include "scene/list.h"
#include "scene/pointselection.h"

namespace omm
{

class Command;
class Project;
class PythonEngine;
class StyleList;
class ObjectTree;
class ToolBox;
class MessageBox;
class HistoryModel;
class Animator;
class NamedColors;
class ColorProperty;

template<typename T> struct SceneStructure;
template<> struct SceneStructure<Object> { using type = ObjectTree; };
template<> struct SceneStructure<Style> { using type = StyleList; };

class Scene : public QObject
{
  Q_OBJECT
public:
  Scene(PythonEngine& python_engine);
  ~Scene();
private:
  Scene(const Scene& other) = delete;
  Scene(Scene&& other) = delete;
private:
  void prepare_reset();
public Q_SLOTS:
  void reset();
private:
  /**
   * holds the last filename this scene was associated to. Is set in `save_as` and `load_from`
   */
  std::string m_filename;


  // === Save/Load ====
public:
  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);
  std::string filename() const;

  static constexpr auto TYPE = "Scene";


  // === Python ===
public:
  PythonEngine& python_engine;


  // === Objects, Tags and Styles and Selections ===
public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  std::set<AbstractPropertyOwner*> selection() const;
  template<typename ItemT> std::set<ItemT*> item_selection() const
  {
    return kind_cast<ItemT>(m_item_selection.at(ItemT::KIND));
  }
  std::set<AbstractPropertyOwner*> property_owners() const;
  std::set<ReferenceProperty*>
  find_reference_holders(const AbstractPropertyOwner& candidate) const;
  std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>>
  find_reference_holders(const std::set<AbstractPropertyOwner*>& candidates) const;
  template<typename T> std::set<T*> find_items(const std::string& name) const;
  bool can_remove( QWidget* parent, std::set<AbstractPropertyOwner*> selection,
                   std::set<Property*>& properties ) const;
  bool remove(QWidget* parent, const std::set<AbstractPropertyOwner*>& selection);
  bool contains(const AbstractPropertyOwner* apo) const;
  PointSelection point_selection;
private:
  std::map<AbstractPropertyOwner::Kind, std::set<AbstractPropertyOwner*>> m_item_selection;
  std::set<AbstractPropertyOwner*> m_selection;
  std::unique_ptr<Object> make_root();


  // === guards ===
public:
  [[nodiscard]] std::unique_ptr<CycleGuard> make_cycle_guard(const Object* guarded);
private:
  std::set<const Object*> m_cycle_guarded_objects;


  // === MessageBox ===
private:
  std::unique_ptr<MessageBox> m_message_box;
public:
  MessageBox& message_box() const { return *m_message_box; }


  // === Objects  ====
private:
  std::unique_ptr<ObjectTree> m_object_tree;
public:
  ObjectTree& object_tree() const { return *m_object_tree; }


  // === Tags ===
public:
  std::set<Tag*> tags() const;
  void evaluate_tags();


  // === Styles ===
public:
  Style& default_style() const;
  StyleList& styles() const { return *m_styles; }
private:
  std::unique_ptr<Style> m_default_style;
  std::unique_ptr<StyleList> m_styles;


  // === Commands ===
private:
  std::unique_ptr<HistoryModel> m_history;
public:
  HistoryModel& history() const { return *m_history; }
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }
  void submit(std::unique_ptr<Command> command);

  // === Tools ===
private:
  std::unique_ptr<ToolBox> m_tool_box;
public:
  ToolBox& tool_box() { return *m_tool_box; }
public Q_SLOTS:
  void update_tool();


  // === Animation ===
private:
  std::unique_ptr<Animator> m_animator;
public:
  Animator& animator() { return *m_animator; }

  // === NamedColors ===
private:
  std::unique_ptr<NamedColors> m_named_colors;
public:
  NamedColors& named_colors() { return *m_named_colors; }
  std::set<ColorProperty*> find_named_color_holders(const std::string& name) const;

};

}  // namespace omm

