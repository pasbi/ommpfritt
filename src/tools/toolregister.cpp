#include "tools/tool.h"
#include "tools/movetool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  LOG(INFO) << "register move tool";
  REGISTER_TOOL(MoveTool);
#undef REGISTER_TOOL
}

}  // namespace omm


