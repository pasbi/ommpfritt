#pragma once

#include <QAbstractItemModel>
#include <QUndoStack>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "cachedgetter.h"
#include "external/json_fwd.hpp"
#include "scene/contextes.h"
#include "scene/cycleguard.h"
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
class MailBox;
class HistoryModel;
class Animator;
class NamedColors;
class ColorProperty;
class Node;

template<typename T> struct SceneStructure;
template<> struct SceneStructure<Object> {
  using type = ObjectTree;
};
template<> struct SceneStructure<Style> {
  using type = StyleList;
};

class Scene : public QObject
{
  Q_OBJECT
public:
  Scene(PythonEngine& python_engine);
  ~Scene() override;

  Scene(Scene&&) = delete;
  Scene(const Scene&) = delete;
  Scene& operator=(Scene&&) = delete;
  Scene& operator=(const Scene&) = delete;

  /**
   * @brief polish do some late initializations
   *  Call this function before using the scene but after the application was initialized.
   */
  void polish();

private:
  void prepare_reset();
public Q_SLOTS:
  void reset();

private:
  /**
   * holds the last filename this scene was associated to. Is set in `save_as` and `load_from`
   */
  QString m_filename;

  // === Save/Load ====
public:
  bool save_as(const QString& filename);
  bool load_from(const QString& filename);
  [[nodiscard]] QString filename() const;

  static constexpr auto TYPE = "Scene";

  // === Python ===
public:
  PythonEngine& python_engine;

  // === Objects, Tags and Styles and Selections ===
public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  [[nodiscard]] std::set<AbstractPropertyOwner*> selection() const;

  template<typename ItemT>
  std::enable_if_t<std::is_same_v<typename ItemT::factory_item_type, ItemT>, std::set<ItemT*>>
  item_selection() const
  {
    return kind_cast<ItemT>(m_item_selection.at(ItemT::KIND));
  }

  template<typename ItemT>
  std::enable_if_t<!std::is_same_v<typename ItemT::factory_item_type, ItemT>, std::set<ItemT*>>
  item_selection() const
  {
    static const auto type_matches = [](const auto* v) { return v->type() == ItemT::TYPE; };
    const auto items = item_selection<typename ItemT::factory_item_type>();
    return type_cast<ItemT*>(::filter_if(items, type_matches));
  }

  void emit_selection_changed_signal();

  [[nodiscard]] std::set<AbstractPropertyOwner*> property_owners() const;
  [[nodiscard]] std::set<ReferenceProperty*>
  find_reference_holders(const AbstractPropertyOwner& candidate) const;
  [[nodiscard]] std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>>
  find_reference_holders(const std::set<AbstractPropertyOwner*>& candidates) const;
  template<typename T> std::set<T*> find_items(const QString& name) const;
  bool can_remove(QWidget* parent,
                  std::set<AbstractPropertyOwner*> selection,
                  std::set<Property*>& properties) const;
  bool remove(QWidget* parent, const std::set<AbstractPropertyOwner*>& selection);
  bool contains(const AbstractPropertyOwner* apo) const;
  PointSelection point_selection;

private:
  std::map<Kind, std::set<AbstractPropertyOwner*>> m_item_selection;
  std::set<AbstractPropertyOwner*> m_selection;
  std::unique_ptr<Object> make_root();

  // === guards ===
public:
  [[nodiscard]] std::unique_ptr<CycleGuard> make_cycle_guard(const Object* guarded);

private:
  std::set<const Object*> m_cycle_guarded_objects;

  // === MailBox ===
private:
  std::unique_ptr<MailBox> m_mail_box;

public:
  [[nodiscard]] MailBox& mail_box() const
  {
    return *m_mail_box;
  }

  // === Objects  ====
private:
  std::unique_ptr<ObjectTree> m_object_tree;

public:
  [[nodiscard]] ObjectTree& object_tree() const
  {
    return *m_object_tree;
  }

  // === Tags ===
public:
  [[nodiscard]] std::set<Tag*> tags() const;
  void evaluate_tags() const;

  // === Nodes ===
public:
  [[nodiscard]] std::set<Node*> collect_nodes() const;
  [[nodiscard]] std::set<Node*> collect_nodes(const std::set<AbstractPropertyOwner*>& owners) const;

  // === Styles ===
public:
  [[nodiscard]] Style& default_style() const;
  [[nodiscard]] StyleList& styles() const
  {
    return *m_styles;
  }

private:
  std::unique_ptr<Style> m_default_style;
  std::unique_ptr<StyleList> m_styles;

  // === Commands ===
private:
  std::unique_ptr<HistoryModel> m_history;

public:
  [[nodiscard]] HistoryModel& history() const
  {
    return *m_history;
  }
  template<typename CommandT, typename... Args> void submit(Args&&... args)
  {
    submit(std::make_unique<CommandT>(std::forward<Args>(args)...));
  }
  void submit(std::unique_ptr<Command> command) const;

  // === Tools ===
private:
  std::unique_ptr<ToolBox> m_tool_box;

public:
  ToolBox& tool_box()
  {
    return *m_tool_box;
  }
public Q_SLOTS:
  void update_tool();

  // === Mode ===
public:
  [[nodiscard]] SceneMode current_mode() const;
  void set_mode(SceneMode mode);

private:
  SceneMode m_mode = SceneMode::Object;

  // === Animation ===
private:
  std::unique_ptr<Animator> m_animator;

public:
  Animator& animator()
  {
    return *m_animator;
  }

  // === NamedColors ===
private:
  std::unique_ptr<NamedColors> m_named_colors;

public:
  NamedColors& named_colors()
  {
    return *m_named_colors;
  }
  [[nodiscard]] std::set<ColorProperty*> find_named_color_holders(const QString& name) const;
};

}  // namespace omm
