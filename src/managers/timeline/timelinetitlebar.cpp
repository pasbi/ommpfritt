#include "managers/timeline/timelinetitlebar.h"
#include <QStyle>
#include "ui_timelinetitlebar.h"
#include <QDockWidget>
#include <QPainter>

namespace omm
{

TimeLineTitleBar::TimeLineTitleBar(Manager& parent)
  : ManagerTitleBar(parent)
  , m_ui(new Ui::TimeLineTitleBar)
{
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  add_widget(std::move(widget));
}

TimeLineTitleBar::~TimeLineTitleBar()
{
}

}  // namespace omm
