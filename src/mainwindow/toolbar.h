#pragma once

#include <QToolBar>

namespace omm
{

class ToolBox;

class ToolBar : public QToolBar
{
  Q_OBJECT
public:
  explicit ToolBar(QWidget* parent, ToolBox& tool_box, const std::vector<std::string>& tools);
  const std::vector<std::string>& tools() const;

private:
  std::vector<std::string> m_tools;
};

}  // namespace omm
