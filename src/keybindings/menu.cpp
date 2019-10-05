#include "keybindings/menu.h"
#include <QEvent>
#include <QMouseEvent>
#include "keybindings/action.h"

namespace omm
{

Menu::Menu(const QString& title) : QMenu(title)
{
  installEventFilter(this);
}

bool Menu::eventFilter(QObject* o, QEvent* e)
{
  if (o == this && e->type() == QEvent::Type::MouseMove) {
    auto* a = dynamic_cast<omm::Action*>(actionAt(static_cast<QMouseEvent*>(e)->pos()));
    if (m_current_highlighted != nullptr) {
      m_current_highlighted->set_highlighted(false);
    }
    if (a != nullptr) {
      a->set_highlighted(true);
    }
    m_current_highlighted = a;
  }
  return QMenu::eventFilter(o, e);
}

}  // namespace omm
