#pragma once

#include "external/json.hpp"
#include "mainwindow/toolbar/toolbaritemmodel.h"
#include <QToolBar>

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
  [[nodiscard]] virtual QString type() const
  {
    return TYPE;
  }
  [[nodiscard]] QString configuration() const;

private:
  void update();
  ToolBarItemModel m_model;
};

}  // namespace omm
