#pragma once

#include <functional>
#include <memory>
#include <glog/logging.h>
#include "maybeowner.h"

namespace omm
{

class Object;


template<typename T, template<typename...> class Wrapper>
class ListContext
{
public:
  ListContext(T& subject, const T* predecessor)
    : subject(subject), predecessor(predecessor)
  {
  }

  const T* predecessor;
  Wrapper<T> subject;
};

template<typename T> using OwningListContext = ListContext<T, MaybeOwner>;

}  // namespace omm
