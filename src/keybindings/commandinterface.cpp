#include "keybindings/commandinterface.h"
#include <QCoreApplication>

namespace omm
{

QString CommandInterface::translate_action_name(const QString& context, const QString& action_name)
{
  const auto* tr_context = ("keybindings/" + context).toUtf8().constData();
  return QCoreApplication::translate(tr_context, action_name.toUtf8().constData());
}

QString CommandInterface::translate_action_name(const QString& action_name) const
{
  return translate_action_name(type(), action_name);
}

}  // namespace omm

