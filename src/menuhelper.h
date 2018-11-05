#pragma once

#include <QMenu>

namespace omm
{

template<typename ReceiverT, typename F>
void action(QMenu& menu, const QString& label, ReceiverT& receiver, F f)
{
  QObject::connect(menu.addAction(label), &QAction::triggered, &receiver, f);
}

template<typename F> void action(QMenu& menu, const QString& label, F f)
{
  QObject::connect(menu.addAction(label), &QAction::triggered, f);
}

}  // namespace omm
