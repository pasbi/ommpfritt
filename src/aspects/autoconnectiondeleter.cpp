#include "autoconnectiondeleter.h"
#include <QObject>

namespace omm
{

AutoConnectionDeleter::~AutoConnectionDeleter()
{
  for (const QMetaObject::Connection& connection : m_connections) {
    QObject::disconnect(connection);
  }
}

void AutoConnectionDeleter::register_connection(const QMetaObject::Connection &connection)
{
  m_connections.push_back(connection);
}

void AutoConnectionDeleter::regc(const QMetaObject::Connection &connection)
{
  register_connection(connection);
}



}  // namespace omm
