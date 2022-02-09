#include "managers/propertymanager/userpropertydialog.h"
#include "aspects/propertyowner.h"
#include "commands/userpropertycommand.h"
#include "logging.h"
#include "propertywidgets/propertyconfigwidget.h"
#include "scene/history/historymodel.h"
#include "scene/scene.h"

namespace omm
{
UserPropertyDialog::UserPropertyDialog(AbstractPropertyOwner& owner,
                                       const std::set<QString>& types,
                                       QWidget* parent)
    : QDialog(parent), m_ui(new Ui::UserPropertyDialog),
      m_property_types(util::transform<std::vector>(types)), m_owner(owner),
      m_user_property_list_model(owner)
{
  m_ui->setupUi(this);
  m_ui->listView->setModel(&m_user_property_list_model);
  const auto select_only_current = [this](const QModelIndex& index) {
    update_property_config_page(m_user_property_list_model.item(index));
    m_ui->listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
  };
  connect(m_ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, select_only_current);

  m_ui->cb_type->addItems(util::transform<QList>(m_property_types, [](const QString& s) {
    return tr(s.toUtf8().constData(), "Property");
  }));
  connect(m_ui->cb_type, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
    m_current_item->configuration.set("type", m_property_types[index]);
    update_property_config_page(m_current_item);
  });
  connect(m_ui->cb_animatable, &QCheckBox::toggled, this, [this](bool checked) {
    m_current_item->configuration.set(Property::ANIMATABLE_POINTER, checked);
  });

  connect(m_ui->pb_add, &QPushButton::clicked, this, [this]() {
    m_user_property_list_model.add_property(m_ui->cb_type->currentText());
    const int n = m_user_property_list_model.rowCount(QModelIndex());
    const QModelIndex index = m_user_property_list_model.index(n - 1, 0);
    m_ui->listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    m_ui->listView->edit(index);
  });
  connect(m_ui->pb_del, &QPushButton::clicked, this, [this]() {
    m_user_property_list_model.del_property(m_ui->listView->currentIndex());
  });
  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &UserPropertyDialog::submit);
  m_ui->cb_type->hide();
  m_ui->cb_animatable->hide();
}

void UserPropertyDialog::submit()
{
  std::vector<std::pair<QString, std::unique_ptr<Property>>> additions;
  std::list<QString> deletions;
  std::map<Property*, PropertyConfiguration> changes;
  const auto keys = m_owner.properties().keys();
  for (const QString& property_key : keys) {
    Property* p = m_owner.property(property_key);
    if (p != nullptr && p->is_user_property() && !m_user_property_list_model.contains(p)) {
      deletions.push_back(property_key);
    }
  }
  additions.reserve(m_user_property_list_model.items().size());
  for (const UserPropertyListItem* item : m_user_property_list_model.items()) {
    if (item->property() != nullptr) {
      changes.insert(std::pair(item->property(), item->configuration));
    } else {
      auto property = Property::make(item->type());
      property->configuration = item->configuration;
      property->set_label(item->label());
      property->set_category(Property::USER_PROPERTY_CATEGROY_NAME);
      QString key = item->label();
      if (key.isEmpty()) {
        key = tr("unnamed");
      }
      for (int i = 0; m_owner.has_property(key); ++i) {
        static constexpr int BASE_DEC = 10;
        static constexpr int WIDTH = 3;
        key = item->label() + QString(".%1").arg(i, WIDTH, BASE_DEC, QChar('0'));
      }
      additions.emplace_back(key, std::move(property));
    }
  }
  additions.shrink_to_fit();
  if (!deletions.empty() || !additions.empty() || !changes.empty()) {
    m_owner.scene()->submit<UserPropertyCommand>(std::vector(deletions.begin(), deletions.end()),
                                                 std::move(additions),
                                                 changes,
                                                 m_owner);
  }
}

void UserPropertyDialog::update_property_config_page(UserPropertyListItem* item)
{
  if (QWidget* current_widget = m_ui->scrollArea->widget(); current_widget != nullptr) {
    current_widget->deleteLater();
  }

  m_current_item = item;

  if (m_current_item == nullptr) {
    m_ui->scrollArea->setWidget(std::make_unique<QWidget>().release());
    m_ui->cb_type->hide();
    m_ui->cb_animatable->hide();
  } else {
    const QString type = m_current_item->type();
    {
      QSignalBlocker blocker(m_ui->cb_type);
      m_ui->cb_type->setEnabled(m_current_item->property() == nullptr);
      m_ui->cb_type->setCurrentText(type);
    }
    {
      QSignalBlocker blocker(m_ui->cb_animatable);
      const bool a = m_current_item->configuration.get(Property::ANIMATABLE_POINTER, true);
      m_ui->cb_animatable->setChecked(a);
    }

    const QString config_widget_type = type + "ConfigWidget";
    auto config_widget = AbstractPropertyConfigWidget::make(config_widget_type);
    connect(config_widget.get(),
            &AbstractPropertyConfigWidget::hidden,
            this,
            [this, config_widget = config_widget.get()]() {
              UserPropertyListItem* item = m_user_property_list_model.item(m_ui->listView->currentIndex());
              config_widget->update(item->configuration);
            });
    m_current_config_widget = config_widget.get();
    config_widget->init(m_current_item->configuration);
    m_ui->scrollArea->setWidget(config_widget.release());
    m_ui->cb_type->show();
    m_ui->cb_animatable->show();
  }
}

}  // namespace omm
