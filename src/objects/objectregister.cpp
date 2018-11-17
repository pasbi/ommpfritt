#include "objects/object.h"
#include "objects/ellipse.h"
#include "objects/empty.h"

namespace omm
{

void register_objects()
{
#define REGISTER_OBJECT(TYPE) Object::register_type<TYPE>(#TYPE);
  REGISTER_OBJECT(Empty);
  REGISTER_OBJECT(Ellipse);
#undef REGISTER_OBJECT
}

}  // namespace omm
