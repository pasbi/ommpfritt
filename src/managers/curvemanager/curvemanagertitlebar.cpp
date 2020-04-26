#include "managers/curvemanager/curvemanagertitlebar.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <memory>
#include "managers/curvemanager/curvemanager.h"
#include "menuhelper.h"

namespace omm
{

CurveManagerTitleBar::CurveManagerTitleBar(CurveManager& parent)
  : ManagerTitleBar(parent)
{
  auto lock_button = std::make_unique<QPushButton>();
  m_lock_button = lock_button.get();
  update_lock_button_icon(m_lock_button->isChecked());
  lock_button->setFixedSize(24, 24);
  lock_button->setCheckable(true);
  connect(lock_button.get(), &QPushButton::toggled, [&parent, this](bool checked) {
    parent.set_locked(checked);
    update_lock_button_icon(checked);
  });

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addStretch(0);
  layout->addWidget(lock_button.release());
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

void CurveManagerTitleBar::update_lock_button_icon(bool checked)
{
  m_lock_button->setIcon(QPixmap::fromImage(QImage(checked ? ":/icons/lock-closed.png"
                                                           : ":/icons/lock-open.png" )));
}


}  // namespace omm
