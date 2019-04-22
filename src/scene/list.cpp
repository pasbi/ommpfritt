#include "scene/list.h"
#include <algorithm>
#include "scene/contextes.h"
#include "renderers/style.h"
#include "tags/tag.h"

namespace
{

template<typename T>
std::vector<std::unique_ptr<T>> copy_items(const std::vector<std::unique_ptr<T>>& items)
{
  return ::transform<std::unique_ptr<T>>(items, [](const auto& i) { return i->clone(); });
}

}  // namespace

namespace omm
{

template<typename T> List<T>::List(const List<T>& other)
  : Structure<T>(), Observed<AbstractStructureObserver<List<T>>>(other)
  ,m_items(copy_items(other.m_items))
{

}

template<typename T> std::set<T*> List<T>::items() const
{
  return ::transform<T*, std::set>(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> std::vector<T*> List<T>::ordered_items() const
{
  return ::transform<T*>(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> T& List<T>::item(size_t i) const
{
  return *m_items[i].get();
}

template<typename T> void List<T>::insert(ListOwningContext<T>& context)
{
  const size_t position = this->insert_position(context.predecessor);
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [position](auto* observer) {
      return observer->acquire_inserter_guard(position);
    }
  );
  m_items.insert(m_items.begin() + static_cast<int>(position), context.subject.release());
  this->invalidate_recursive();
}

template<typename T> void List<T>::remove(ListOwningContext<T>& context)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this, &context](auto* observer) {
      return observer->acquire_remover_guard(position(context.subject));
    }
  );
  context.subject.capture(::extract(m_items, context.subject.get()));
  this->invalidate_recursive();
}

template<typename T> std::unique_ptr<T> List<T>::remove(T& item)
{
  // item could be `const T&`, however, that would break compatibility with `Tree::remove`.
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this, &item](auto* observer){ return observer->acquire_remover_guard(position(item)); }
  );
  auto extracted_item = ::extract(m_items, item);
  this->invalidate_recursive();
  return  extracted_item;
}

template<typename T> size_t List<T>::position(const T& item) const
{
  const auto it = std::find_if(m_items.begin(), m_items.end(), [&item](const auto& uptr) {
    return uptr.get() == &item;
  });

  assert(it != m_items.end());
  auto i = std::distance(m_items.begin(), it);
  assert(i >= 0);
  return static_cast<std::size_t>(i);
}

template<typename T> const T* List<T>::predecessor(const T& item) const
{
  const auto pos = this->position(item);
  if (pos == 0) {
    return nullptr;
  } else {
    return m_items.at(pos - 1).get();
  }
}

template<typename T> void List<T>::move(ListMoveContext<T>& context)
{
  assert(context.is_valid());
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );

  std::unique_ptr<T> item = ::extract(m_items, context.subject.get());
  const auto i = m_items.begin() + static_cast<int>(this->insert_position(context.predecessor));
  m_items.insert(i, std::move(item));
  this->invalidate_recursive();
}

template<typename T>
std::vector<std::unique_ptr<T>> List<T>::set(std::vector<std::unique_ptr<T>> items)
{
  const auto style_guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [](auto* observer) { return observer->acquire_reseter_guard(); }
  );
  auto old_items = std::move(m_items);
  m_items = std::move(items);
  this->invalidate_recursive();
  return old_items;
}

template<typename T> size_t List<T>::size() const
{
  return m_items.size();
}

template<typename T> void List<T>::invalidate() { }

template<typename T> bool List<T>::contains(const T &item) const
{
  return m_items.end() != std::find_if(m_items.begin(), m_items.end(), [&item](const auto& i) {
    return i.get() == &item;
  });
}

template class List<Style>;
template class List<Tag>;

}  // namespace omm
