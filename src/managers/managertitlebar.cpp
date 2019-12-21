#include "managers/managertitlebar.h"
#include <memory>
#include <QPushButton>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "managers/manager.h"

namespace omm
{

ManagerTitleBar::ManagerTitleBar(Manager& manager)
  : m_manager(manager)
{
  auto pb_normal = std::make_unique<QPushButton>();
  pb_normal->setMaximumSize(QSize(12, 12));
  pb_normal->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
  connect(pb_normal.get(), &QPushButton::clicked, this, [&manager]() {
    manager.setFloating(!manager.isFloating());
  });

  auto pb_close = std::make_unique<QPushButton>();
  pb_close->setMaximumSize(QSize(12, 12));
  pb_close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  connect(pb_close.get(), SIGNAL(clicked()), &manager, SLOT(close()));

  auto hlayout = std::make_unique<QHBoxLayout>();
  hlayout->addWidget(pb_normal.release());
  hlayout->addWidget(pb_close.release());
  hlayout->setSpacing(2);

  auto vlayout = std::make_unique<QVBoxLayout>();
  vlayout->addLayout(hlayout.release());
  vlayout->setSpacing(0);
  vlayout->addStretch(0);

  auto layout = std::make_unique<QHBoxLayout>();
  layout->addLayout(vlayout.release());
  layout->setContentsMargins(0, 0, 2, 0);
  m_layout = layout.get();
  setLayout(layout.release());
}

void ManagerTitleBar::add_widget(std::unique_ptr<QWidget> widget)
{
  m_layout->insertWidget(0, widget.release());
  m_layout->setStretch(0, 1);
  m_layout->setStretch(1, 0);
}

std::unique_ptr<QPushButton> ManagerTitleBar::make_lock_button() const
{
  auto lock_button = std::make_unique<QPushButton>();
  const auto update_lock_button_icon = [&btn=*lock_button](bool checked) {
    btn.setIcon(QPixmap::fromImage(QImage(checked ? ":/icons/LockedLock.png"
                                                  : ":/icons/OpenLock.png" )));
  };
  update_lock_button_icon(lock_button->isChecked());
  lock_button->setFixedSize(24, 24);
  lock_button->setCheckable(true);
  connect(lock_button.get(), &QPushButton::toggled, [this, update_lock_button_icon](bool checked) {
    m_manager.set_locked(checked);
    update_lock_button_icon(checked);
  });
  return lock_button;
}

void ManagerTitleBar::apply_standard_layout(std::vector<std::unique_ptr<QWidget>> widgets)
{
  auto container = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addStretch(0);
  for (auto&& widget : widgets) {
    layout->addWidget(widget.release());
  }
  container->setLayout(layout.release());
  add_widget(std::move(container));
}

QSize ManagerTitleBar::sizeHint() const
{
  return QSize(0, 24);
}

}  // namespace omm
