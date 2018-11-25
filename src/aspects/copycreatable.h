#pragma once

#include "aspects/serializable.h"
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
  : public virtual Serializable
  , public AbstractFactory<std::string, T, CtorArgs...>
{
public:
  std::unique_ptr<T> copy() const
  {
    std::stringstream stream;
    auto serializer = CopyCreatableDetail::make_copy_serialzier(stream);
    auto deserializer = CopyCreatableDetail::make_copy_deserialzier(stream);
    auto copy = this->make(this->type());
    static constexpr auto POINTER = "copy";
    {
      this->serialize(*serializer, POINTER);
    }
    {
      copy->deserialize(*deserializer, POINTER);
    }
    return copy;
  }
};

}  // omm
