#include "tags/tag.h"

namespace omm
{

void register_tags()
{
#define REGISTER_TAG(TYPE) Tag::register_type<TYPE>(#TYPE);
  // REGISTER_TAG(Tag);
#undef REGISTER_TAG
}

}  // namespace omm


