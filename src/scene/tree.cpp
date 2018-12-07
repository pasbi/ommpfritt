#include "scence/tree.h"
#include "scene/contextes.h"

namespace omm
{

template<typename T> T& Tree<T>::root() const
{
  return *m_root;
}

template<typename T> void Tree<T>::move(TreeMoveContext<T> context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  const auto guards = Observed<AbstractObjectTreeObserver>::transform<Guard>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::insert(TreeOwningContext<T>& context)
{
  assert(context.subject.owns());

  const auto guards = Observed<AbstractObjectTreeObserver>::transform<Guard>(
    [&context] (auto* observer) {
      return observer->acquire_inserter_guard(context.parent, context.get_insert_position());
    }
  );
  context.parent.get().adopt(context.subject.release(), context.predecessor);

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::remove(TreeOwningContext<T>& context)
{
  assert(!context.subject.owns());

  const auto guards = Observed<AbstractObjectTreeObserver>::transform<Guard>(
    [&context](auto* observer) { return observer->acquire_remover_guard(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T>
std::unique_ptr<Object> Tree<T>::replace_root(std::unique_ptr<T> new_root)
{
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  return old_root;
}

template<typename T> std::set<T*> Tree<T>::TGetter::compute() const
{
  return m_self.root().all_descendants();
}

template<typename T>  std::set<T*> Tree<T>::selected_objects() const
{
  return ::filter_if(objects(), is_selected<Object>);
}

template<typename T> const T* Tree<T>::predecessor(const T& sibling) const
{
  return sibling.predecessor();  // TODO move implementation from T to here.
}

}  // namespace
