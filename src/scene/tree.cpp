#include "scene/tree.h"
#include "scene/contextes.h"

namespace
{
// TODO same in List
using Guard = std::unique_ptr<AbstractRAIIGuard>;
}  // namespace

namespace omm
{

template<typename T> Tree<T>::Tree(std::unique_ptr<T> root)
  : m_root(std::move(root))
{
}

template<typename T> Tree<T>::~Tree()
{
}

template<typename T> T& Tree<T>::root() const
{
  return *m_root;
}

template<typename T> void Tree<T>::move(TreeMoveContext<T> context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  const auto guards = observed_type::template transform<Guard>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::insert(TreeOwningContext<T>& context)
{
  assert(context.subject.owns());

  const auto guards = observed_type::template transform<Guard>(
    [&context] (auto* observer) {
      return observer->acquire_inserter_guard(context.parent, context.get_insert_position());
    }
  );
  context.parent.get().adopt(context.subject.release(), context.predecessor);

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::insert(std::unique_ptr<T> item, T& parent)
{
  size_t n = parent.children().size();

  {
    const auto guards = observed_type::template transform<Guard>(
      [&parent, n] (auto* observer) { return observer->acquire_inserter_guard(parent, n); }
    );

    parent.adopt(std::move(item));
  }

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::remove(TreeOwningContext<T>& context)
{
  assert(!context.subject.owns());

  const auto guards = observed_type::template transform<Guard>(
    [&context](auto* observer) { return observer->acquire_remover_guard(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));

  items.invalidate();
  // tags.invalidate();  // TODO
}

template<typename T>
std::unique_ptr<T> Tree<T>::replace_root(std::unique_ptr<T> new_root)
{
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  return old_root;
}

template<typename T> std::set<T*> Tree<T>::TGetter::compute() const
{
  return Tree<T>::TGetter::m_self.root().all_descendants();
}

template<typename T>  std::set<T*> Tree<T>::selected_items() const
{
  // TODO same in List, Scene
  const auto is_selected = [](const auto* t) { return t->is_selected(); };
  return ::filter_if(items(), is_selected);
}

template<typename T> const T* Tree<T>::predecessor(const T& sibling) const
{
  return sibling.predecessor();  // TODO move implementation from T to here.
}

template class Tree<Object>;

}  // namespace
