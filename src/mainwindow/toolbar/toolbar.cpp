#include "mainwindow/toolbar/toolbar.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include "tools/toolbox.h"
#include <QApplication>
#include <QContextMenuEvent>
#include <QToolButton>

namespace omm
{
ToolBar::ToolBar()
{
  setAttribute(Qt::WA_DeleteOnClose);
  update();
}

ToolBar::ToolBar(const QString& configuration)
{
  setAttribute(Qt::WA_DeleteOnClose);
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
