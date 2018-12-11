#pragma once

#include "aspects/serializable.h"
#include "abstractfactory.h"
#include "serializers/abstractserializer.h"

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

    const auto serialize = [&]() {
      auto serializer = CopyableDetail::make_copy_serialzier(stream);
      this->serialize(*serializer, POINTER);
      return serializer->serialized_references();
      // it is very important that `serializer` becomes
      // destroyed before `deserializer` is created.
    };

    const auto serialized_references = serialize();

    {
      auto deserializer = CopyableDetail::make_copy_deserialzier(stream);
      deserializer->add_references(serialized_references);
      raw_copy->deserialize(*deserializer, POINTER);
    }
    return raw_copy;
  }
};

}  // omm
