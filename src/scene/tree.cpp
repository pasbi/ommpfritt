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

template<typename T> void Tree<T>::move(TreeMoveContext<T>& context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  const auto guards = observed_type::template transform<Guard>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);

  m_item_cache_is_dirty = true;
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

  m_item_cache_is_dirty = true;
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

  m_item_cache_is_dirty = true;
  // tags.invalidate();  // TODO
}

template<typename T> void Tree<T>::remove(TreeOwningContext<T>& context)
{
  assert(!context.subject.owns());

  const auto guards = observed_type::template transform<Guard>(
    [&context](auto* observer) { return observer->acquire_remover_guard(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));

  m_item_cache_is_dirty = true;
  // tags.invalidate();  // TODO
}

template<typename T>
std::unique_ptr<T> Tree<T>::replace_root(std::unique_ptr<T> new_root)
{
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  return old_root;
}

template<typename T> std::set<T*> Tree<T>::items() const
{
  if (m_item_cache_is_dirty) {
    m_item_cache_is_dirty = false;
    m_item_cache = root().all_descendants();
  }
  return m_item_cache;
}

template<typename T> size_t Tree<T>::position(const T& item) const
{
  return item.position();
}

template<typename T> const T* Tree<T>::predecessor(const T& sibling) const
{
  assert(!sibling.is_root());
  const auto pos = position(sibling);
  if (pos == 0) {
    return nullptr;
  } else {
    return &sibling.parent().child(pos - 1);
  }
}

template class Tree<Object>;

}  // namespace
