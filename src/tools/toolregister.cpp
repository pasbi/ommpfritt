#include "tools/tool.h"
#include "tools/movetool.h"
#include "tools/scaletool.h"
#include "tools/rotatetool.h"

namespace omm
{

void register_tools()
{
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(MoveTool);
  REGISTER_TOOL(ScaleTool);
  REGISTER_TOOL(RotateTool);
#undef REGISTER_TOOL
}

}  // namespace omm


