#include "tags/styletag.h"

namespace omm
{

void register_tags()
{
#define REGISTER_TAG(TYPE) Tag::register_type<TYPE>(#TYPE);
  REGISTER_TAG(StyleTag);
#undef REGISTER_TAG
}

}  // namespace omm


