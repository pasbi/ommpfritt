#include "objects/objecttransformation.h"
#include "external/json.hpp"

namespace omm
{

nlohmann::json ObjectTransformation::to_json() const
{
  return "";
}

std::ostringstream& operator<<(std::ostringstream& ostream, const ObjectTransformation& t)
{
  // ostream << std::string("transformation");
  return ostream;
}

}  // namespace omm
