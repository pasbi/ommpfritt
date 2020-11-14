#pragma once

#include <QMenu>

namespace omm
{
template<typename ReceiverT, typename F>
QAction& action(QMenu& menu, const QString& label, ReceiverT& receiver, F f)
{
  auto& action = *menu.addAction(label);
  QObject::connect(&action, &QAction::triggered, &receiver, f);
  return action;
}

template<typename F> QAction& action(QMenu& menu, const QString& label, F f)
{
  auto& action = *menu.addAction(label);
  QObject::connect(&action, &QAction::triggered, f);
  return action;
}

}  // namespace omm
