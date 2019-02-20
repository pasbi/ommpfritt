#include "mainwindow/toolbar.h"
#include "tools/toolbox.h"

namespace omm
{

ToolBar::ToolBar(QWidget* parent, ToolBox& tool_box, const std::vector<std::string>& tools)
  : QToolBar(tr("ToolBar"), parent)
  , m_tools(tools)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName("ToolBar");
  for (auto&& tool_class : tools) {
    auto& tool = tool_box.tool(tool_class);
    auto* action = addAction(tool.icon(), QString::fromStdString(tool.type()));
    connect(action, &QAction::triggered, [tool_class, &tool_box]() {
      tool_box.set_active_tool(tool_class);
    });
  }
}

const std::vector<std::string>& ToolBar::tools() const
{
  return m_tools;
}

}  // namespace omm
