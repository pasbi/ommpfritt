#include "managers/propertymanager/propertymanagertitlebar.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <memory>
#include "managers/propertymanager/propertymanager.h"
#include "menuhelper.h"

namespace omm
{

PropertyManagerTitleBar::PropertyManagerTitleBar(PropertyManager& parent)
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

  auto open_user_property_dialog_button = std::make_unique<QPushButton>();
  m_open_user_properties_dialog_button = open_user_property_dialog_button.get();
  open_user_property_dialog_button->setEnabled(false);
  open_user_property_dialog_button->setText("U");
  open_user_property_dialog_button->setFixedSize(24, 24);
  connect(open_user_property_dialog_button.get(), &QPushButton::clicked, this, [this, &parent]() {
    auto dialog = UserPropertyDialog(*m_first_selected, &parent);
    dialog.exec();
  });

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addStretch(0);
  layout->addWidget(lock_button.release());
  layout->addWidget(open_user_property_dialog_button.release());
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

void PropertyManagerTitleBar::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (selection.size() == 1) {
    m_first_selected = *selection.begin();
    m_open_user_properties_dialog_button->setEnabled(true);
  } else {
    m_first_selected = nullptr;
    m_open_user_properties_dialog_button->setEnabled(false);
  }
}

void PropertyManagerTitleBar::update_lock_button_icon(bool checked)
{
  m_lock_button->setIcon(QPixmap::fromImage(QImage(checked ? ":/icons/LockedLock.png"
                                                           : ":/icons/OpenLock.png" )));
}


}  // namespace omm
