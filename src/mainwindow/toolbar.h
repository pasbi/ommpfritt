#pragma once

#include <QToolBar>

namespace omm
{

class ToolBox;

class ToolBar : public QToolBar
{
  Q_OBJECT
public:
  explicit ToolBar(QWidget* parent, ToolBox& tool_box, const std::vector<QString>& tools);
  const std::vector<QString>& tools() const;

private:
  std::vector<QString> m_tools;
};

}  // namespace omm
