#include "managers/timeline/timelinetitlebar.h"
#include <QStyle>
#include "ui_timelinetitlebar.h"
#include <QDockWidget>
#include <QPainter>

namespace omm
{

TimeLineTitleBar::TimeLineTitleBar(QDockWidget& parent) : m_ui(new Ui::TimeLineTitleBar)
{
  m_ui->setupUi(this);
  m_ui->pb_normal->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
  m_ui->pb_close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  connect(m_ui->pb_close, SIGNAL(clicked()), &parent, SLOT(close()));
  connect(m_ui->pb_normal, &QPushButton::clicked, this, [&parent]() {
    parent.setFloating(!parent.isFloating());
  });
}

TimeLineTitleBar::~TimeLineTitleBar()
{
}

}  // namespace omm
