#include "objects/objecttransformation.h"
#include "external/json.hpp"

namespace omm
{

std::ostringstream& operator<<(std::ostringstream& ostream, const ObjectTransformation& t)
{
  // ostream << std::string("transformation");
  return ostream;
}

}  // namespace omm
