#include "tools/tool.h"
#include "tools/brushselecttool.h"
#include "tools/knifetool.h"
#include "tools/pathtool.h"
#include "tools/selectpointstool.h"
#include "tools/selectobjectstool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(SelectObjectsTool)
  REGISTER_TOOL(SelectPointsTool)
  REGISTER_TOOL(BrushSelectTool)
  REGISTER_TOOL(KnifeTool)
  REGISTER_TOOL(PathTool)
#undef REGISTER_TOOL
}

}  // namespace omm


