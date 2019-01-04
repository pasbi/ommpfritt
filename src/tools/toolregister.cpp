#include "tools/tool.h"
#include "tools/pathtool.h"
#include "tools/objectstools/movetool.h"
#include "tools/objectstools/scaletool.h"
#include "tools/objectstools/rotatetool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(MoveTool);
  REGISTER_TOOL(ScaleTool);
  REGISTER_TOOL(RotateTool);
  REGISTER_TOOL(PathTool);
#undef REGISTER_TOOL
}

}  // namespace omm


