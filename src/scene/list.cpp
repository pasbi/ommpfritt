#include "scene/list.h"
#include <algorithm>
#include "scene/contextes.h"
#include "renderers/style.h"
#include "tags/tag.h"
#include "aspects/propertyowner.h"

namespace
{

template<typename T>
std::vector<std::unique_ptr<T>> copy_items(const std::vector<std::unique_ptr<T>>& items)
{
  return ::transform<std::unique_ptr<T>>(items, [](const auto& i) { return i->clone(); });
}

template<typename T>
void register_items(const std::vector<std::unique_ptr<T>>& items, omm::List<T>& list)
{
  if constexpr (std::is_base_of_v<omm::AbstractPropertyOwner, T>) {
    for (auto&& item : items) {
      item->register_observer(&list);
    }
  }
}

template<typename T>
void unregister_items(const std::vector<std::unique_ptr<T>>& items, omm::List<T>& list)
{
  if constexpr (std::is_base_of_v<omm::AbstractPropertyOwner, T>) {
    for (auto&& item : items) {
      item->unregister_observer(&list);
    }
  }
}

}  // namespace

namespace omm
{

template<typename T> List<T>::List(const List<T>& other)
  : Structure<T>()
  , m_items(copy_items(other.m_items))
{
  register_items(m_items, *this);
}

template<typename T> List<T>::~List()
{
  unregister_items(m_items, *this);
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
  const int row = this->insert_position(context.predecessor);
  m_items.insert(m_items.begin() + static_cast<int>(row), context.subject.release());
  if constexpr (std::is_base_of_v<AbstractPropertyOwner, T>) {
    context.get_subject().register_observer(this);
  }
}

template<typename T> void List<T>::remove(ListOwningContext<T>& context)
{
  if constexpr (std::is_base_of_v<AbstractPropertyOwner, T>) {
    context.get_subject().unregister_observer(this);
  }
  context.subject.capture(::extract(m_items, context.subject.get()));
}

template<typename T> std::unique_ptr<T> List<T>::remove(T& item)
{
  auto extracted_item = ::extract(m_items, item);
  if constexpr (std::is_base_of_v<AbstractPropertyOwner, T>) {
    item.unregister_observer(this);
  }
  return extracted_item;
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
  std::unique_ptr<T> item = ::extract(m_items, context.subject.get());
  const auto i = m_items.begin() + static_cast<int>(this->insert_position(context.predecessor));
  m_items.insert(i, std::move(item));
}

template<typename T>
std::vector<std::unique_ptr<T>> List<T>::set(std::vector<std::unique_ptr<T>> items)
{
  unregister_items(m_items, *this);
  auto old_items = std::move(m_items);
  m_items = std::move(items);
  register_items(m_items, *this);
  return old_items;
}

template<typename T> size_t List<T>::size() const
{
  return m_items.size();
}

template<typename T> bool List<T>::contains(const T &item) const
{
  return m_items.end() != std::find_if(m_items.begin(), m_items.end(), [&item](const auto& i) {
    return i.get() == &item;
  });
}

template<typename T>
void List<T>::on_change(AbstractPropertyOwner *apo, int what, Property *property,
                        std::set<const void*> trace)
{
  Q_UNUSED(apo)
  Q_UNUSED(what)
  Q_UNUSED(property)
}

template class List<Style>;
template class List<Tag>;

}  // namespace omm
