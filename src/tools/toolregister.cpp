#include "tools/tool.h"
#include "tools/selecttool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(SelectObjectsTool);
  REGISTER_TOOL(SelectPointsTool);
#undef REGISTER_TOOL
}

}  // namespace omm


