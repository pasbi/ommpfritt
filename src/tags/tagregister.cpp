#include "tags/styletag.h"
#include "tags/scripttag.h"

namespace omm
{

void register_tags()
{
#define REGISTER_TAG(TYPE) Tag::register_type<TYPE>(#TYPE);
  REGISTER_TAG(StyleTag);
  REGISTER_TAG(ScriptTag);
#undef REGISTER_TAG
}

}  // namespace omm


