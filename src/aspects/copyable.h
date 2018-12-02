#pragma once

#include "aspects/serializable.h"
#include "abstractfactory.h"

namespace omm
{

namespace CopyableDetail
{

std::unique_ptr<AbstractSerializer> make_copy_serialzier(std::ostream& ostream);
std::unique_ptr<AbstractDeserializer> make_copy_deserialzier(std::istream& istream);

}  // namespace CopyableDetail

template<typename T>
class Copyable : public virtual Serializable
{
public:
  virtual std::unique_ptr<T> copy() const = 0;

protected:
  std::unique_ptr<T> copy(std::unique_ptr<T> raw_copy) const
  {
    std::stringstream stream;
    static constexpr auto POINTER = "copy";
    {
      auto serializer = CopyableDetail::make_copy_serialzier(stream);
      this->serialize(*serializer, POINTER);
    }
    {
      auto deserializer = CopyableDetail::make_copy_deserialzier(stream);
      raw_copy->deserialize(*deserializer, POINTER);
    }
    return raw_copy;
  }
};

template<typename T>
class SimpleCopyable : public Copyable<T>
{
public:
  std::unique_ptr<T> copy() const override
  {
    return Copyable<T>::copy(std::make_unique<T>());
  }
};

}  // omm
