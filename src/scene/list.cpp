#include "aspects/listelement.h"
#include <algorithm>

namespace omm
{

template<typename T> std::set<T*> List<T>::items() const
{
  return ::transform<T*, std::set>(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> T& List<T>::item(size_t i) const
{
  return *m_items[i].get();
}

template<typename T> std::set<T*> List<T>::selected_styles() const
{
  return ::filter_if(items(), is_selected<T>);
}

template<typename T> void List<T>::insert(std::unique_ptr<T> item)
{
  const auto guards = Observed<AbstractTListObserver>::transform<Guard>(
    [this](auto* observer){ return observer->acquire_inserter_guard(m_items.size()); }
  );
  m_items.push_back(std::move(item));
  selection_changed();
}

template<typename T> void List<T>::insert(TListOwningContext& item)
{
  size_t position = item.predecessor() == nullptr ? 0 : item.predecessor()->position() + 1;
  const auto guards = Observed<AbstractTListObserver>::transform<Guard>(
    [this, position](auto* observer){ return observer->acquire_inserter_guard(position); }
  );
  m_items.insert(m_items.begin() + position, item.subject.release());
  selection_changed();
}

template<typename T> void List<T>::remove(TListOwningContext& style_context)
{
  const size_t position = style_context.subject.position();
  const auto guards = Observed<AbstractTListObserver>::transform<Guard>(
    [this, position](auto* observer){ return observer->acquire_remover_guard(position); }
  );
  style_context.subject.capture(::extract(m_items, style_context.subject.reference()));
  selection_changed();
}

template<typename T> std::unique_ptr<T> List<T>::remove(T& item)
{
  ;
  const auto guards = Observed<AbstractTListObserver>::transform<Guard>(
    [this, &item](auto* observer){ return observer->acquire_remover_guard(item.position()); }
  );
  return ::extract(m_items, item);
}

template<typename T> size_t List<T>::position(const T& item) const
{
  const auto pos = std::find_if(m_items.begin(), m_items.end(), [&item](const auto& uptr) {
    return uptr == &item;
  });

  assert(pos != m_items->end());
  return std::distance(pos, m_items->begin());
}

template<typename T> const T* List<T>::predecessor(const T& item) const
{
  const auto pos = this->position(item);
  if (pos == 0) {
    return nullptr;
  } else {
    return &m_items.at(pos - 1);
  }
}

}  // namespace omm
