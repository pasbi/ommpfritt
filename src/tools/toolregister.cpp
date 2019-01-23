#include "tools/tool.h"
#include "tools/selecttool.h"
#include "tools/brushselecttool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(SelectObjectsTool);
  REGISTER_TOOL(SelectPointsTool);
  REGISTER_TOOL(BrushSelectTool);
#undef REGISTER_TOOL
}

}  // namespace omm


