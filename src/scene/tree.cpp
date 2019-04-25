#include "scene/tree.h"
#include "scene/contextes.h"
#include <type_traits>

namespace omm
{

template<typename T> Tree<T>::Tree(std::unique_ptr<T> root, Scene*)
  : Structure<T>()
  , m_root(std::move(root))
{
}

template<typename T> T& Tree<T>::root() const
{
  return *m_root;
}

template<typename T> bool Tree<T>::contains(const T& t) const
{
  static_assert(std::is_base_of_v<TreeElement<T>, T>);
  const auto& root = static_cast<const TreeElement<T>&>(*m_root);
  return root.is_ancestor_of(t);
}

template<typename T> void Tree<T>::move(TreeMoveContext<T>& context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );
  auto item = old_parent.repudiate(context.subject);
  const auto pos = this->insert_position(context.predecessor);
  context.parent.get().adopt(std::move(item), pos);
  m_item_cache_is_dirty = true;
  Q_EMIT this->structure_changed();
}

template<typename T> void Tree<T>::insert(TreeOwningContext<T>& context)
{
  assert(context.subject.owns());

  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&context, this] (auto* observer) {
      const auto pos = this->insert_position(context.predecessor);
      return observer->acquire_inserter_guard(context.parent, pos);
    }
  );
  const auto pos = this->insert_position(context.predecessor);
  context.parent.get().adopt(context.subject.release(), pos);
  m_item_cache_is_dirty = true;
  Q_EMIT this->structure_changed();
}

template<typename T> void Tree<T>::remove(TreeOwningContext<T>& context)
{
  assert(!context.subject.owns());

  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&context](auto* observer) { return observer->acquire_remover_guard(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));
  m_item_cache_is_dirty = true;
  Q_EMIT this->structure_changed();
}

template<typename T> std::unique_ptr<T> Tree<T>::remove(T& t)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&t](auto* observer) { return observer->acquire_remover_guard(t); }
  );
  assert(!t.is_root());
  auto item = t.parent().repudiate(t);
  m_item_cache_is_dirty = true;
  Q_EMIT this->structure_changed();
  return item;
}

template<typename T>
std::unique_ptr<T> Tree<T>::replace_root(std::unique_ptr<T> new_root)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [](auto* observer) { return observer->acquire_reseter_guard(); }
  );
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  m_item_cache_is_dirty = true;
  Q_EMIT this->structure_changed();
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
