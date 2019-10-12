#include "preferences/uicolorspage.h"
#include "ui_uicolorspage.h"
#include "preferences/uicolorstreeviewdelegate.h"

namespace omm
{

UiColorsPage::UiColorsPage(UiColors& colors)
  : m_ui(new Ui::UiColorsPage)
{
  m_ui->setupUi(this);
  m_ui->treeView->set_delegate(std::make_unique<UiColorsTreeViewDelegate>());
  m_ui->treeView->set_model(colors);
}

UiColorsPage::~UiColorsPage()
{
}

void UiColorsPage::about_to_accept()
{
  m_ui->treeView->transfer_editor_data_to_model();
}

void UiColorsPage::about_to_reject()
{
}

}  // namespace omm
