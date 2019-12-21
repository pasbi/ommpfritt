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
  auto open_user_property_dialog_button = std::make_unique<QPushButton>();
  m_open_user_properties_dialog_button = open_user_property_dialog_button.get();
  open_user_property_dialog_button->setEnabled(false);
  open_user_property_dialog_button->setText("U");
  open_user_property_dialog_button->setFixedSize(24, 24);
  connect(open_user_property_dialog_button.get(), &QPushButton::clicked, this, [this, &parent]() {
    auto dialog = UserPropertyDialog(*m_first_selected, &parent);
    dialog.exec();
  });

  std::vector<std::unique_ptr<QWidget>> widgets;
  widgets.reserve(2);
  widgets.push_back(make_lock_button());
  widgets.push_back(std::move(open_user_property_dialog_button));
  apply_standard_layout(std::move(widgets));
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



}  // namespace omm
