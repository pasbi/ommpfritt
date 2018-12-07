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
#include "scene/cachedgetter.h"
#include "scene/structure.h"
#include "scene/contextes_fwd.h"
#include "scene/abstractstructureobserver.h"

namespace omm
{

template<typename T>
class Tree : public Structure<T>, public Observed<AbstractStructureObserver<Tree<T>>>
{
public:
  Tree();
  ~Tree();

  class TGetter : public CachedGetter<std::set<T*>, Tree<T>&>
  {
  private:
    explicit TGetter(Tree<T>& scene) : CachedGetter<std::set<T*>, Tree<T>&>(scene) {}
    friend class Tree<T>;
  protected:
    std::set<T*> compute() const override;
  };

public:
  void insert(std::unique_ptr<T> object, T& parent);
  void insert(TreeOwningContext<T>& context);
  void move(TreeMoveContext<T> context);
  void remove(TreeOwningContext<T>& context);
  bool can_move_object(const TreeMoveContext<T>& new_context) const;
  const TGetter items = TGetter(*this);
  std::set<T*> selected_objects() const;
  std::unique_ptr<T> replace_root(std::unique_ptr<T> new_root);
  T& root() const;
  const T* predecessor(const T& sibling) const override;
private:
  std::unique_ptr<T> m_root;

};

}  // namespace omm

