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
#include "scene/contextes.h"
#include "scene/abstractselectionobserver.h"
#include "scene/abstractobjecttreeobserver.h"
#include "scene/abstractstylelistobserver.h"
#include "scene/cachedgetter.h"

namespace omm
{

class Command;
class Object;
class Project;

class Scene
  : public Observed<AbstractObjectTreeObserver>
  , public Observed<AbstractSelectionObserver>
  , public Observed<AbstractStyleListObserver>
{
public:
  Scene();
  ~Scene();

  ObjectView root_view();

  void reset();
  template<class T>
  class TGetter : public CachedGetter<std::set<T*>, Scene&>
  {
  private:
    explicit TGetter(Scene& scene) : CachedGetter<std::set<T*>, Scene&>(scene) {}
    friend class Scene;
  protected:
    std::set<T*> compute() const override;
  };

  // === Objects ====
public:
  void insert_object(std::unique_ptr<Object> object, Object& parent);
  void insert_object(ObjectTreeOwningContext& context);
  void move_object(ObjectTreeMoveContext context);
  void remove_object(ObjectTreeOwningContext& context);
  bool can_move_object(const ObjectTreeMoveContext& new_context) const;
  const TGetter<Object> objects = TGetter<Object>(*this);
  std::set<Object*> selected_objects() const;
  std::unique_ptr<Object> replace_root(std::unique_ptr<Object> new_root);
  Object& root() const;
private:
  std::unique_ptr<Object> m_root;

  // === Tags  ======
public:
  Tag& attach_tag(Object& owner, std::unique_ptr<Tag> tag, const Tag* predecessor);
  Tag& attach_tag(Object& owner, std::unique_ptr<Tag> tag);
  std::unique_ptr<Tag> detach_tag(Object& owner, Tag& tag);
  const TGetter<Tag> tags = TGetter<Tag>(*this);
  std::set<Tag*> selected_tags() const;

  // === Styles  ====
public:
  void insert_style(std::unique_ptr<Style> style);
  void insert_style(StyleListOwningContext& style);
  std::unique_ptr<Style> remove_style(Style& style);  // TODO remove?
  void remove_style(StyleListOwningContext& style);
  std::set<Style*> styles() const;
  Style& style(size_t i) const;
  std::set<Style*> selected_styles() const;
  Style& default_style() const;
  size_t position(const Style& style) const;
private:
  std::unique_ptr<Style> m_default_style;
  std::vector<std::unique_ptr<Style>> m_styles;

  // === Objects, Tags and Styles ===
public:
  std::set<AbstractPropertyOwner*> selection() const;
  std::set<AbstractPropertyOwner*> property_owners() const;
  std::set<ReferenceProperty*>
  find_reference_holders(const AbstractPropertyOwner& candidate) const;

  // === Selection ===
public:
  void clear_selection();
  void selection_changed();

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

};

}  // namespace omm

