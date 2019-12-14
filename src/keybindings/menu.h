#pragma once

#include <QMenu>

namespace omm
{

class Action;

// TODO I think this class is obsolete ...
class Menu : public QMenu
{
public:
  explicit Menu(const QString& title);

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  Action* m_current_highlighted = nullptr;
};

}  // namespace omm
