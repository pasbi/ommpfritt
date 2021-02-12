#include "managers/managertitlebar.h"
#include "managers/manager.h"
#include "mainwindow/iconprovider.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <memory>

namespace omm
{
ManagerTitleBar::ManagerTitleBar(Manager& manager) : m_manager(manager)
{
  static constexpr int MAX_BUTTON_SIZE = 12;
  auto pb_normal = std::make_unique<QPushButton>();
  pb_normal->setMaximumSize(QSize(MAX_BUTTON_SIZE, MAX_BUTTON_SIZE));
  pb_normal->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
  connect(pb_normal.get(), &QPushButton::clicked, this, [&manager]() {
    manager.setFloating(!manager.isFloating());
  });

  auto pb_close = std::make_unique<QPushButton>();
  pb_close->setMaximumSize(QSize(MAX_BUTTON_SIZE, MAX_BUTTON_SIZE));
  pb_close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  connect(pb_close.get(), &QPushButton::clicked, &manager, &Manager::close);

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
  const auto update_lock_button_icon = [&btn = *lock_button](bool checked) {
    btn.setIcon(IconProvider::pixmap(checked ? "lock-closed" : "lock-open"));
  };
  update_lock_button_icon(lock_button->isChecked());
  static constexpr int MAX_BUTTON_SIZE = 24;
  lock_button->setFixedSize(MAX_BUTTON_SIZE, MAX_BUTTON_SIZE);
  lock_button->setCheckable(true);
  connect(lock_button.get(), &QPushButton::toggled, [this, update_lock_button_icon](bool checked) {
    m_manager.set_locked(checked);
    update_lock_button_icon(checked);
  });
  return lock_button;
}

QSize ManagerTitleBar::sizeHint() const
{
  static constexpr int MAX_BUTTON_SIZE = 24;
  return QSize(0, MAX_BUTTON_SIZE);
}

}  // namespace omm
