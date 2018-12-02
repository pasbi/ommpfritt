#include "aspects/copycreatable.h"
#include "serializers/jsonserializer.h"

namespace omm
{

namespace CopyableDetail
{

std::unique_ptr<AbstractSerializer> make_copy_serialzier(std::ostream& ostream)
{
  return AbstractSerializer::make("JSONSerializer", ostream);
}

std::unique_ptr<AbstractDeserializer> make_copy_deserialzier(std::istream& istream)
{
  return AbstractDeserializer::make("JSONDeserializer", istream);
}

}  // namespace CopyableDetail

}  // omm
