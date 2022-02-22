#include "scene/list.h"
#include "aspects/propertyowner.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include "tags/tag.h"
#include <algorithm>

namespace
{
template<typename Ts>
auto copy_items(const Ts& items)
{
  return util::transform(items, [](const auto& i) { return i->clone(); });
}

}  // namespace

namespace omm
{
template<typename T>
List<T>::List(const List<T>& other) : Structure<T>(), m_items(copy_items(other.m_items))
{
}

template<typename T> std::set<T*> List<T>::items() const
{
  return util::transform<std::set>(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> std::deque<T*> List<T>::ordered_items() const
{
  return util::transform(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> T& List<T>::item(std::size_t i) const
{
  return *m_items[i].get();
}

template<typename T> void List<T>::insert(ListOwningContext<T>& context)
{
  const int row = this->insert_position(context.predecessor);
  m_items.insert(m_items.begin() + static_cast<int>(row), context.subject.release());
}

template<typename T> void List<T>::remove(ListOwningContext<T>& context)
{
  context.subject.capture(::extract(m_items, context.subject.get()));
}

template<typename T> std::unique_ptr<T> List<T>::remove(T& item)
{
  auto extracted_item = ::extract(m_items, item);
  return extracted_item;
}

template<typename T> std::size_t List<T>::position(const T& item) const
{
  const auto it = std::find_if(m_items.begin(), m_items.end(), [&item](const auto& uptr) {
    return uptr.get() == &item;
  });

  assert(it != m_items.end());
  const auto i = std::distance(m_items.begin(), it);
  return static_cast<std::size_t>(i);
}

template<typename T> std::size_t List<T>::insert_position(const T* predecessor) const
{
  if (predecessor == nullptr) {
    return 0;
  } else {
    const auto it = std::find_if(m_items.cbegin(), m_items.cend(), [predecessor](const auto& i) {
      return i.get() == predecessor;
    });
    if (it == m_items.cend()) {
      return m_items.size();
    } else {
      return std::distance(m_items.begin(), it) + 1;
    }
  }
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
  std::unique_ptr<T> item = ::extract(m_items, context.subject.get());
  const auto i = m_items.begin() + static_cast<int>(this->insert_position(context.predecessor));
  m_items.insert(i, std::move(item));
}

template<typename T>
std::deque<std::unique_ptr<T>> List<T>::set(std::deque<std::unique_ptr<T>> items)
{
  auto old_items = std::move(m_items);
  m_items = std::move(items);
  return old_items;
}

template<typename T> std::size_t List<T>::size() const
{
  return m_items.size();
}

template<typename T> bool List<T>::contains(const T& item) const
{
  return m_items.end() != std::find_if(m_items.begin(), m_items.end(), [&item](const auto& i) {
    return i.get() == &item;
  });
}

template class List<Style>;
template class List<Tag>;

}  // namespace omm
