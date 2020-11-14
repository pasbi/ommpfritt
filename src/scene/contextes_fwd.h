#pragma once

#include "maybeowner.h"

namespace omm
{
template<typename T, template<typename...> class Wrapper> class ListContext;
template<typename T, template<typename...> class Wrapper> class TreeContext;

template<typename T, template<typename, template<typename...> class> class ContextT>
class MoveContext;

template<typename T, template<typename, template<typename...> class> class ContextT>
using OwningContext = ContextT<T, MaybeOwner>;

template<typename T> class ListOwningContext;

template<typename T> class TreeMoveContext;
template<typename T> class ListMoveContext;

class Object;
using ObjectTreeMoveContext = TreeMoveContext<Object>;
class ObjectTreeOwningContext;

class Style;
using StyleListOwningContext = ListOwningContext<Style>;
using StyleListMoveContext = ListMoveContext<Style>;

class Tag;
using TagListOwningContext = ListOwningContext<Tag>;
using TagListMoveContext = ListMoveContext<Tag>;

template<typename T> struct Contextes;

template<> struct Contextes<Object> {
  using Move = ObjectTreeMoveContext;
  using Owning = ObjectTreeOwningContext;
};

template<> struct Contextes<Style> {
  using Move = StyleListMoveContext;
  using Owning = StyleListOwningContext;
};

template<> struct Contextes<Tag> {
  using Move = TagListMoveContext;
  using Owning = TagListOwningContext;
};

}  // namespace omm
