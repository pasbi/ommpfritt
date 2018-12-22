#include "objects/object.h"
#include "objects/ellipse.h"
#include "objects/empty.h"
#include "objects/instance.h"
#include "objects/cloner.h"

namespace omm
{

void register_objects()
{
#define REGISTER_OBJECT(TYPE) Object::register_type<TYPE>(#TYPE);
  REGISTER_OBJECT(Empty);
  REGISTER_OBJECT(Ellipse);
  REGISTER_OBJECT(Instance);
  REGISTER_OBJECT(Cloner);
#undef REGISTER_OBJECT
}

}  // namespace omm
