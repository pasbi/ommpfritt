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
  auto menu_bar = std::make_unique<QMenuBar>();
  auto user_properties_menu = menu_bar->addMenu(QObject::tr("user properties", "PropertyManager"));
  const auto exec_user_property_dialog = [&parent, this]() {
    auto dialog = UserPropertyDialog(*m_first_selected, &parent);
    dialog.exec();
  };

  m_manage_user_properties_action = &action( *user_properties_menu,
                                             QObject::tr("edit ...", "PropertyManager"),
                                             exec_user_property_dialog );
  m_manage_user_properties_action->setEnabled(false);

  auto lock_button = std::make_unique<QPushButton>();
  lock_button->setFixedSize(24, 24);
  lock_button->setText("L");
  lock_button->setCheckable(true);
  connect(lock_button.get(), &QPushButton::toggled, [&parent](bool checked) {
    parent.set_locked(checked);
  });

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(menu_bar.release());
  layout->addStretch(0);
  layout->addWidget(lock_button.release());
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

void PropertyManagerTitleBar::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (selection.size() == 1) {
    m_first_selected = *selection.begin();
    m_manage_user_properties_action->setEnabled(true);
  } else {
    m_first_selected = nullptr;
    m_manage_user_properties_action->setEnabled(false);
  }
}


}  // namespace omm
