#include "objects/object.h"
#include "objects/ellipse.h"
#include "objects/empty.h"
#include "objects/imageobject.h"
#include "objects/instance.h"
#include "objects/cloner.h"
#include "objects/path.h"
#include "objects/proceduralpath.h"
#include "objects/view.h"

namespace omm
{

void register_objects()
{
#define REGISTER_OBJECT(TYPE) Object::register_type<TYPE>(#TYPE);
  REGISTER_OBJECT(Empty);
  REGISTER_OBJECT(Ellipse);
  REGISTER_OBJECT(ImageObject);
  REGISTER_OBJECT(Instance);
  REGISTER_OBJECT(Cloner);
  REGISTER_OBJECT(Path);
  REGISTER_OBJECT(ProceduralPath);
  REGISTER_OBJECT(View);
#undef REGISTER_OBJECT
}

}  // namespace omm
