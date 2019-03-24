#include "managers/propertymanager/userpropertymanager/userpropertydialog.h"
#include <memory>
#include <functional>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMenuBar>
#include <QPushButton>
#include "menuhelper.h"
#include "managers/propertymanager/userpropertymanager/propertyitem.h"
#include "propertywidgets/propertyconfigwidget.h"
#include "aspects/propertyowner.h"

namespace
{

auto make_list_widget()
{
  // TODO when pressing ctrl during drag, the cursor suggests that copying is enabled.
  // however drop always issues moving the item, even with ctrl being pressed.
  auto list_widget = std::make_unique<QListWidget>();
  list_widget->setDragEnabled(true);
  list_widget->setDragDropMode(QAbstractItemView::InternalMove);
  list_widget->setDefaultDropAction(Qt::MoveAction);
  list_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
  return list_widget;
}

struct ButtonBox
{
  QAbstractButton* add_button;
  QAbstractButton* remove_button;
  QAbstractButton* accept_button;
  QAbstractButton* reject_button;
  std::unique_ptr<QLayout> layout;
};

auto make_button_box()
{
  const auto button_size = QSize(22, 22); // TODO shouldn't that depend on DPI or so?
  ButtonBox button_box;
  const auto buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
  auto dialog_button_box = std::make_unique<QDialogButtonBox>(buttons);
  button_box.accept_button = dialog_button_box->button(QDialogButtonBox::Ok);
  button_box.reject_button = dialog_button_box->button(QDialogButtonBox::Cancel);

  auto add_button = std::make_unique<QPushButton>("+");
  add_button->setFixedSize(button_size);
  button_box.add_button = add_button.get();

  auto remove_button = std::make_unique<QPushButton>("-");
  remove_button->setFixedSize(button_size);
  button_box.remove_button = remove_button.get();

  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(add_button.release());
  layout->addWidget(remove_button.release());
  layout->addStretch();
  layout->addWidget(dialog_button_box.release());
  button_box.layout = std::move(layout);
  return button_box;
}

}  // namespace

namespace omm
{

UserPropertyDialog::UserPropertyDialog(QWidget* parent, AbstractPropertyOwner& property_owner)
  : QDialog(parent)
  , m_property_owner(property_owner)
{
  auto* main_layout = std::make_unique<QVBoxLayout>(this).release();
  main_layout->setMenuBar(std::make_unique<QMenuBar>(this).release());

  m_layout = std::make_unique<QHBoxLayout>().release();
  main_layout->addLayout(m_layout);

  auto list_widget = make_list_widget();
  m_list_widget = list_widget.get();
  m_layout->addWidget(list_widget.release());

  m_right_column = nullptr;

  connect(m_list_widget, &QListWidget::currentItemChanged, [this](auto* item) {
    on_current_item_changed(static_cast<PropertyItem*>(item));
  });

  auto button_box = make_button_box();
  connect(button_box.accept_button, &QAbstractButton::clicked, this, &UserPropertyDialog::accept);
  connect(button_box.reject_button, &QAbstractButton::clicked, this, &UserPropertyDialog::reject);
  connect(button_box.add_button, &QPushButton::clicked, this, &UserPropertyDialog::new_item);
  connect(button_box.remove_button, &QPushButton::clicked,
          this, &UserPropertyDialog::remove_selected_item );
  main_layout->addLayout(button_box.layout.release());

  for (auto&& property : property_owner.properties().values()) {
    if (property->is_user_property()) {
      m_list_widget->addItem(std::make_unique<PropertyItem>(property->clone()).release());
    }
  }
}

void UserPropertyDialog::new_item()
{
  auto* const item = std::make_unique<PropertyItem>().release();
  m_list_widget->addItem(item);
  m_list_widget->setCurrentItem(item);
}

void UserPropertyDialog::remove_selected_item()
{
  const auto selected_items = m_list_widget->selectedItems();
  auto selected_rows = ::transform<int>(selected_items, [this](const auto* item) {
    return m_list_widget->row(item);
  });
  std::sort(selected_rows.begin(), selected_rows.end(), std::greater<int>());
  for (auto row : selected_rows) {
    delete m_list_widget->takeItem(row);
  }
}

void UserPropertyDialog::on_current_item_changed(QListWidgetItem*)
{
  if (m_right_column != nullptr) { m_right_column->deleteLater(); }
  m_right_column = nullptr;
  const auto current_item = this->current_item();
  if (current_item != nullptr) {
    const auto property_type = current_item->property().type();
    const auto pcwt = property_type + "ConfigWidget";
    auto& current_property = current_item->property();
    m_right_column = AbstractPropertyConfigWidget::make(pcwt, this, current_property).release();
    m_layout->takeAt(1);
    m_layout->addWidget(m_right_column);
    connect( m_right_column, &AbstractPropertyConfigWidget::property_type_changed,
             this, &UserPropertyDialog::on_current_item_type_changed );
    connect( m_right_column, &AbstractPropertyConfigWidget::property_label_changed,
             this, &UserPropertyDialog::on_current_item_label_changed);

  }
}

void UserPropertyDialog::on_current_item_label_changed()
{
  current_item()->setText(QString::fromStdString(current_item()->property().label()));
}

void UserPropertyDialog::on_current_item_type_changed(const std::string& type)
{
  current_item()->set_property_type(type);
  on_current_item_changed(current_item());
}

PropertyItem* UserPropertyDialog::current_item() const
{
  return static_cast<PropertyItem*>(m_list_widget->currentItem());
}

std::unique_ptr<UserPropertyConfigCommand>
UserPropertyDialog::make_user_property_config_command() const
{
  std::vector<std::unique_ptr<Property>> properties;
  const auto n = m_list_widget->count();
  properties.reserve(static_cast<std::size_t>(n));
  for (int i = 0; i < n; ++i) {
    properties.push_back(static_cast<PropertyItem*>(m_list_widget->item(i))->property().clone());
  }
  return std::make_unique<UserPropertyConfigCommand>(m_property_owner, std::move(properties));
}

}  // namespace omm
