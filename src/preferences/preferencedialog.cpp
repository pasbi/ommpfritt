#include "preferences/preferencedialog.h"
#include "ui_preferencedialog.h"
#include "preferences/uicolorspage.h"
#include "mainwindow/application.h"
#include "preferences/keybindingspage.h"

namespace omm
{

PreferenceDialog::PreferenceDialog() : m_ui(new Ui::PreferenceDialog)
{
  m_ui->setupUi(this);
  Application& app = Application::instance();
  register_preference_page(nullptr, tr("Ui Colors"),
                           std::make_unique<UiColorsPage>(app.ui_colors));
  register_preference_page(nullptr, tr("Keyindings"),
                           std::make_unique<KeyBindingsPage>(app.key_bindings));

}

PreferenceDialog::~PreferenceDialog()
{
}

QTreeWidgetItem* PreferenceDialog::
register_preference_page(QTreeWidgetItem* parent, const QString& label,
                         std::unique_ptr<QWidget> page)
{
  auto item = std::make_unique<QTreeWidgetItem>();
  item->setText(0, label);
  QTreeWidgetItem& ref = *item;
  if (parent == nullptr) {
    m_ui->treeWidget->addTopLevelItem(item.release());
  } else {
    parent->addChild(item.release());
  }

  connect(m_ui->treeWidget, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
    m_ui->stackedWidget->setCurrentIndex(m_page_map.at(item));
  });

  connect(m_ui->treeWidget, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* item) {
    m_ui->stackedWidget->setCurrentIndex(m_page_map.at(item));
  });

  m_page_map.insert(std::pair(&ref, m_ui->stackedWidget->count()));
  m_ui->stackedWidget->addWidget(page.release());
  return &ref;
}

}  // namespace
