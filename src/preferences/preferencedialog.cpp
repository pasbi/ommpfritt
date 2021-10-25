#include "preferences/preferencedialog.h"
#include "main/application.h"
#include "preferences/generalpage.h"
#include "preferences/keybindingspage.h"
#include "preferences/preferencepage.h"
#include "preferences/preferences.h"
#include "preferences/uicolorspage.h"
#include "preferences/viewportpage.h"
#include "ui_preferencedialog.h"
#include "syncpalettedecorator.h"

namespace omm
{
PreferenceDialog::PreferenceDialog() : m_ui(new Ui::PreferenceDialog)
{
  m_ui->setupUi(this);
  Application& app = Application::instance();

  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &PreferenceDialog::accept);
  connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &PreferenceDialog::reject);

  register_preference_page(nullptr, tr("General"), std::make_unique<GeneralPage>(*app.preferences));
  register_preference_page(nullptr,
                           tr("Viewport"),
                           std::make_unique<ViewportPage>(*app.preferences));
  register_preference_page(nullptr, tr("Ui Colors"), std::make_unique<UiColorsPage>(*app.ui_colors));
  register_preference_page(nullptr,
                           tr("Keyindings"),
                           std::make_unique<KeyBindingsPage>(*app.key_bindings));
}

PreferenceDialog::~PreferenceDialog() = default;

void PreferenceDialog::accept()
{
  for (auto&& [_, page] : m_page_map) {
    page->about_to_accept();
  }
  QDialog::accept();
}

void PreferenceDialog::reject()
{
  for (auto&& [_, page] : m_page_map) {
    page->about_to_reject();
  }
  QDialog::reject();
}

QTreeWidgetItem* PreferenceDialog::register_preference_page(QTreeWidgetItem* parent,
                                                            const QString& label,
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

  SyncPaletteTreeWidgetItemDecorator::decorate(ref);

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

}  // namespace omm
