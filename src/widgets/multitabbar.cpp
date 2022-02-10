#include "widgets/multitabbar.h"
#include "common.h"
#include "logging.h"
#include <QApplication>
#include <QDragEnterEvent>
#include <QPushButton>
#include <QStyle>

namespace omm
{
MultiTabBar::MultiTabBar() : m_layout(std::make_unique<QHBoxLayout>())
{
  setLayout(m_layout.get());
  m_change_current_on_drag_delay_timer.setInterval(
      style()->styleHint(QStyle::SH_TabBar_ChangeCurrentDelay));
  m_change_current_on_drag_delay_timer.setSingleShot(true);
  connect(&m_change_current_on_drag_delay_timer, &QTimer::timeout, this, [this]() {
    assert(m_drag_activated_index >= 0);
    auto selection = current_indices();
    if (extend_selection()) {
      selection.insert(m_drag_activated_index);
    } else {
      selection = {m_drag_activated_index};
    }
    set_current_indices(selection);
    Q_EMIT current_indices_changed(selection);
  });
}

bool MultiTabBar::extend_selection()
{
  return (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) != 0u;
}

void MultiTabBar::add_tab(const QString& text)
{
  auto tab = std::make_unique<QPushButton>(text);
  tab->setCheckable(true);
  connect(tab.get(),
          &QAbstractButton::clicked,
          this,
          [tab = tab.get(), i = m_tabs.size(), this](bool checked) {
            if (!checked && current_indices().empty()) {
              set_current_indices({});
            } else if (!extend_selection()) {
              set_current_indices({static_cast<int>(i)});
            }
            // NOLINTNEXTLINE(readability-misleading-indentation)
            Q_EMIT current_indices_changed(current_indices());
          });
  tab->installEventFilter(this);
  tab->setAcceptDrops(true);
  m_layout->addWidget(tab.get());
  m_tabs.push_back(std::move(tab));
}

std::set<int> MultiTabBar::current_indices() const
{
  std::set<int> indices;
  for (std::size_t i = 0; i < m_tabs.size(); ++i) {
    if (m_tabs[i]->isChecked()) {
      indices.insert(i);
    }
  }
  return indices;
}

void MultiTabBar::clear()
{
  for (const auto& tab : m_tabs) {
    m_layout->removeWidget(tab.get());
  }
  m_tabs.clear();
}

bool MultiTabBar::eventFilter(QObject* o, QEvent* e)
{
  switch (e->type()) {
  case QEvent::DragEnter: {
    const auto it = std::find_if(m_tabs.begin(), m_tabs.end(), [o](const auto& tab) {
      return tab.get() == o;
    });
    if (it != m_tabs.end()) {
      m_drag_activated_index = std::distance(m_tabs.begin(), it);
    }
    m_change_current_on_drag_delay_timer.start();
    e->accept();
  } break;
  case QEvent::DragMove:
    e->ignore();
    break;
  case QEvent::DragLeave:
  case QEvent::Drop:
    m_change_current_on_drag_delay_timer.stop();
    m_drag_activated_index = -1;
    e->ignore();
    break;
  default:
    break;
  }
  return QWidget::eventFilter(o, e);
}

void MultiTabBar::set_current_indices(const std::set<int>& indices)
{
  if (indices.empty()) {
    for (auto&& button : m_tabs) {
      button->setChecked(true);
    }
  } else {
    for (std::size_t i = 0; i < m_tabs.size(); ++i) {
      QAbstractButton* button = m_tabs[i].get();
      QSignalBlocker blocker(button);
      button->setChecked(indices.contains(i));
    }
  }
}

}  // namespace omm
