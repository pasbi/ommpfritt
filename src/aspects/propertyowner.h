#pragma once

#include "aspects/abstractpropertyowner.h"

namespace omm
{
template<Kind kind_> class PropertyOwner : public AbstractPropertyOwner
{
public:
  PropertyOwner(Scene* scene) : AbstractPropertyOwner(kind_, scene)
  {
  }
  static constexpr Kind KIND = kind_;
};

template<typename T, typename S> T kind_cast(S s)
{
  if (s != nullptr && s->kind == std::remove_pointer_t<T>::KIND) {
    return static_cast<T>(s);
  } else {
    return nullptr;
  }
}

template<typename T, template<typename...> class ContainerT>
ContainerT<T*> kind_cast(const ContainerT<AbstractPropertyOwner*>& ss)
{
  auto casted = util::transform(ss, [](auto* s) { return kind_cast<T*>(s); });
  std::erase_if(casted, [](const auto* p) { return p == nullptr; });
  return casted;
}

template<typename T, template<typename...> class ContainerT>
ContainerT<AbstractPropertyOwner*> down_cast(const ContainerT<T*>& ss)
{
  return util::transform<AbstractPropertyOwner*>(ss);
}

}  // namespace omm
