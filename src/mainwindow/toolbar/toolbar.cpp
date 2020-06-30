#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include <QContextMenuEvent>
#include <QToolButton>
#include "tools/toolbox.h"
#include "mainwindow/application.h"
#include "mainwindow/iconprovider.h"
#include <QApplication>

namespace omm
{

ToolBar::ToolBar()
{
  setAttribute(Qt::WA_DeleteOnClose);
}

ToolBar::ToolBar(const QString& configuration)
  : ToolBar()
{
  m_model.reset(configuration);
  update();
}

QString ToolBar::configuration() const
{
  return m_model.encode_str();
}

void ToolBar::update()
{
  clear();
  m_model.populate(*this);
  auto spacer = std::make_unique<QWidget>();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  addWidget(spacer.release());
  connect(addAction(tr("E")), &QAction::triggered, [this]() {
    const auto safe = m_model.encode_str();
    ToolBarDialog dialog(m_model);
    if (dialog.exec() == QDialog::Accepted) {
      update();
    } else {
      m_model.reset(safe);
    }
  });
}

}  // namespace omm
