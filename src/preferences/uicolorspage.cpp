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

  std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> delegates(3);
  for (std::size_t i = 0; i < 3; ++i) {
    delegates.at(i) = std::make_unique<UiColorsTreeViewDelegate>();
  }
  m_ui->treeView->set_model(colors, std::move(delegates));
  connect(m_ui->pb_saveas, &QPushButton::clicked, this, [this]() {
    m_colors.save_to_file("/tmp/colors.cfg");
  });
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
}

}  // namespace omm
