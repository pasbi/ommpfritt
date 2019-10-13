#include "preferences/preferencedialog.h"
#include "ui_preferencedialog.h"
#include "preferences/uicolorspage.h"
#include "mainwindow/application.h"
#include "preferences/keybindingspage.h"
#include "preferences/preferencepage.h"
#include "preferences/generalpage.h"
#include <QApplication>

namespace omm
{

PreferenceDialog::PreferenceDialog() : m_ui(new Ui::PreferenceDialog)
{
  m_ui->setupUi(this);
  Application& app = Application::instance();

  connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  register_preference_page(nullptr, tr("General"),
                           std::make_unique<GeneralPage>());
  register_preference_page(nullptr, tr("Ui Colors"),
                           std::make_unique<UiColorsPage>(app.ui_colors));
  register_preference_page(nullptr, tr("Keyindings"),
                           std::make_unique<KeyBindingsPage>(app.key_bindings));
}

PreferenceDialog::~PreferenceDialog()
{
}

void PreferenceDialog::accept()
{
  for (auto&& [ _, page ] : m_page_map) {
    page->about_to_accept();
  }
  QDialog::accept();
}

void PreferenceDialog::reject()
{
  for (auto&& [ _, page ] : m_page_map) {
    page->about_to_reject();
  }
  QDialog::reject();
}

QTreeWidgetItem* PreferenceDialog::
register_preference_page(QTreeWidgetItem* parent, const QString& label,
                         std::unique_ptr<PreferencePage> page)
{
  auto item = std::make_unique<QTreeWidgetItem>();
  item->setText(0, label);
  QTreeWidgetItem& ref = *item;
  if (parent == nullptr) {
    m_ui->treeWidget->addTopLevelItem(item.release());
  } else {
    parent->addChild(item.release());
  }

  connect(qApp, &QGuiApplication::paletteChanged, m_ui->treeWidget, [&ref](const QPalette& p) {
    ref.setForeground(0, p.color(QPalette::Active, QPalette::WindowText));
  });
  ref.setForeground(0, qApp->palette().color(QPalette::Active, QPalette::WindowText));

  connect(m_ui->treeWidget, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
    m_ui->stackedWidget->setCurrentWidget(m_page_map.at(item));
  });

  connect(m_ui->treeWidget, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* item) {
    m_ui->stackedWidget->setCurrentWidget(m_page_map.at(item));
  });

  m_page_map.insert(std::pair(&ref, page.get()));
  m_ui->stackedWidget->addWidget(page.release());
  return &ref;
}

}  // namespace
