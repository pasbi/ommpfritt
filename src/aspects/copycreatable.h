#pragma once

#include "aspects/copyable.h"
#include "abstractfactory.h"

namespace omm
{

namespace CopyCreatableDetail
{

std::unique_ptr<AbstractSerializer> make_copy_serialzier(std::ostream& ostream);
std::unique_ptr<AbstractDeserializer> make_copy_deserialzier(std::istream& istream);

}  // namespace CopyCreatableDetail

template<typename T, typename... CtorArgs>
class CopyCreatable
  : public Copyable<T>
  , public AbstractFactory<std::string, T, CtorArgs...>
{
public:
  virtual std::unique_ptr<T> copy() const = 0;
};

}  // omm
