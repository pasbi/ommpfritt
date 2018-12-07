// #include "aspects/listelement.h"
// #include <algorithm>

// namespace omm
// {

// template<typename T> ListElement<T>::ListElement(const List<T>* list)
//   : m_list(list)
// {
// }

// template<typename T> size_t ListElement<T>::position() const
// {
//   assert(m_list != nullptr);
//   const auto pos = std::find_if(m_list->begin(), m_list->end(), [this](const auto& uptr) {
//     return uptr == this;
//   });

//   assert(pos != m_list->end());
//   return std::distance(pos, m_list->begin());
// }

// template<typename T> const T* ListElement<T>::predecessor() const
// {
//   const auto pos = this->position();
//   if (pos == 0) {
//     return nullptr;
//   } else {
//     return &m_list.at(pos - 1);
//   }
// }

// }  // namespace omm
