#include "preferences/uicolorspage.h"
#include "ui_uicolorspage.h"
#include "preferences/uicolorstreeviewdelegate.h"
#include "preferences/uicolors.h"
#include <QApplication>

namespace omm
{

UiColorsPage::UiColorsPage(UiColors& colors)
  : m_ui(new Ui::UiColorsPage)
  , m_colors(colors)
{
  m_ui->setupUi(this);
  m_skins.push_back(std::pair(tr("dark"), ":/skins/ui-colors-dark.cfg"));
  m_skins.push_back(std::pair(tr("light"), ":/skins/ui-colors-light.cfg"));
  update_combobox();
  connect(m_ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(set_default_values(int)));

  std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> delegates(3);
  for (std::size_t i = 0; i < 3; ++i) {
    delegates.at(i) = std::make_unique<UiColorsTreeViewDelegate>();
  }
  m_ui->treeView->set_model(colors, std::move(delegates));
  connect(m_ui->pb_saveas, &QPushButton::clicked, this, [this]() {
    m_colors.save_to_file("/tmp/colors.cfg");
  });

  connect(m_ui->pb_reset, &QPushButton::clicked, this, [this]() {
    m_colors.reset();
  });
  m_colors.store();
}

UiColorsPage::~UiColorsPage()
{
}

void UiColorsPage::about_to_accept()
{
  m_ui->treeView->transfer_editor_data_to_model();
  qApp->setPalette(m_colors.make_palette());
}

void UiColorsPage::about_to_reject()
{
  m_colors.restore();
}

void UiColorsPage::set_default_values(int index)
{
  m_colors.load_from_file(m_skins.at(index).second);
}

void UiColorsPage::update_combobox()
{
  QSignalBlocker blocker(m_ui->comboBox);
  m_ui->comboBox->clear();
  for (auto&& [ name, _ ] : m_skins) {
    m_ui->comboBox->addItem(name);
  }
}

}  // namespace omm
