#pragma once

#include "logging.h"
#include <QCommandLineParser>
#include <iostream>

namespace omm
{
class SubcommandLineParser : public QCommandLineParser
{
public:
  static constexpr auto COMMAND_TREE = "tree";
  static constexpr auto STATUS_CODE = "status";
  static constexpr auto COMMAND_RENDER = "render";
  explicit SubcommandLineParser(const QStringList& args);
  explicit SubcommandLineParser();
  [[nodiscard]] QString command() const
  {
    return m_command;
  }

  template<typename T> T get(const QString& name) const;
  template<typename T> T get(const QString& name, const T& default_value) const
  {
    if (const QString value = this->value(name); value.isEmpty()) {
      return default_value;
    } else {
      return get<T>(name);
    }
  }

  static constexpr auto FRAMENUMBER_PLACEHOLDER = '#';

private:
  const QString m_appname;
  const QString m_command;
  static void print_help();
};

}  // namespace omm
