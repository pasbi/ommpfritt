#include "tags/styletag.h"
#include "tags/scripttag.h"
#include "tags/pathtag.h"

namespace omm
{

void register_tags()
{
#define REGISTER_TAG(TYPE) Tag::register_type<TYPE>(#TYPE);
  REGISTER_TAG(StyleTag);
  REGISTER_TAG(ScriptTag);
  REGISTER_TAG(PathTag);
#undef REGISTER_TAG
}

}  // namespace omm


