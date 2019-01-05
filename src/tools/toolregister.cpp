#include "tools/tool.h"
#include "tools/itemtools/movetool.h"
#include "tools/itemtools/scaletool.h"
#include "tools/itemtools/rotatetool.h"
#include "tools/itemtools/selecttool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(ObjectMoveTool);
  REGISTER_TOOL(ObjectScaleTool);
  REGISTER_TOOL(ObjectRotateTool);
  REGISTER_TOOL(SelectObjectsTool);

  REGISTER_TOOL(PointMoveTool);
  REGISTER_TOOL(PointScaleTool);
  REGISTER_TOOL(PointRotateTool);
  REGISTER_TOOL(SelectPointsTool);
#undef REGISTER_TOOL
}

}  // namespace omm


