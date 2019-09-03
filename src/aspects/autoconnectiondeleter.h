#pragma once

#include <list>
#include <QMetaObject>

namespace omm
{

class AutoConnectionDeleter
{
protected:
  AutoConnectionDeleter() = default;
  AutoConnectionDeleter(const AutoConnectionDeleter&) {}  // don't copy the list.
  virtual ~AutoConnectionDeleter();

  /**
   * @brief register_connection registers a connection to be disconnected when this is deleted.
   * @param connection
   */
  void register_connection(const QMetaObject::Connection& connection);

  /**
   * @brief reg shortcut for @code register_connection
   * @param connection
   */
  void regc(const QMetaObject::Connection& connection);

private:
  std::list<QMetaObject::Connection> m_connections;
};

}  // namespace omm
