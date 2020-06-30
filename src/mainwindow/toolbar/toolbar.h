#pragma once

#include <QToolBar>
#include "external/json.hpp"
#include "mainwindow/toolbar/toolbaritemmodel.h"

namespace omm
{

class ToolBox;

class ToolBar : public QToolBar
{
  Q_OBJECT
public:
  explicit ToolBar(const QString& configuration);
  explicit ToolBar();

  static constexpr auto TYPE = "ToolBar";
  virtual QString type() const { return TYPE; }
  QString configuration() const;

private:
  void update();
  ToolBarItemModel m_model;
};

}  // namespace omm
