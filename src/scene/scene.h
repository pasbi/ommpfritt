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
#include "scene/cachedgetter.h"
#include "scene/list.h"
#include "scene/tree.h"

namespace omm
{

class Command;
class Object;
class Project;

class Scene
  : public Observed<AbstractSelectionObserver>
  , public List<Style>
  , public Tree<Object>
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

  // === Tags  ======
public:
  Tag& attach_tag(Object& owner, std::unique_ptr<Tag> tag, const Tag* predecessor);
  Tag& attach_tag(Object& owner, std::unique_ptr<Tag> tag);
  std::unique_ptr<Tag> detach_tag(Object& owner, Tag& tag);
  const TGetter<Tag> tags = TGetter<Tag>(*this);
  std::set<Tag*> selected_tags() const;

  // === Styles  ====
public:
  Style& default_style() const;
private:
  std::unique_ptr<Style> m_default_style;

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

