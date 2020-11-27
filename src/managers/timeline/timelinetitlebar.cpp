#include "managers/timeline/timelinetitlebar.h"
#include "ui_timelinetitlebar.h"
#include <QDockWidget>
#include <QPainter>
#include <QStyle>

namespace omm
{
TimeLineTitleBar::TimeLineTitleBar(Manager& parent)
    : ManagerTitleBar(parent), m_ui(new Ui::TimeLineTitleBar)
{
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  add_widget(std::move(widget));
}

TimeLineTitleBar::~TimeLineTitleBar() = default;

}  // namespace omm
