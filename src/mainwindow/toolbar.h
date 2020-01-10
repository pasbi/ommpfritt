#pragma once

#include <QToolBar>

namespace omm
{

class ToolBox;

class ToolBar : public QToolBar
{
  Q_OBJECT
public:
  explicit ToolBar(const QString& tools = "");
  QString tools() const  { return m_tools; }
  void configure(const QString& tools);

  static constexpr auto TYPE = "ToolBar";
  virtual QString type() const { return TYPE; }

private:
  QString m_tools;
};

}  // namespace omm
