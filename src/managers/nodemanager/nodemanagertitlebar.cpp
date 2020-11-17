#include "managers/nodemanager/nodemanagertitlebar.h"
#include "managers/nodemanager/nodemanager.h"
#include "menuhelper.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <memory>

namespace omm
{
NodeManagerTitleBar::NodeManagerTitleBar(NodeManager& parent) : ManagerTitleBar(parent)
{
  auto lock_button = make_lock_button();

  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addStretch(0);
  layout->addWidget(lock_button.release());
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

}  // namespace omm
