#pragma once

#include <QString>
#include <map>
#include <variant>

namespace omm
{
class StringInterpolation
{
public:
  static constexpr auto START = '{';
  static constexpr auto END = '}';
  static constexpr auto SEPARATOR = ':';

  class InvalidFormatException : public std::runtime_error
  {
  public:
    InvalidFormatException(const QString& what) : std::runtime_error(what.toStdString())
    {
    }
  };

  using value_variant = std::variant<int, QString>;
  using map_type = std::map<QString, value_variant>;
  StringInterpolation(const QString& pattern, const map_type& values);
  operator QString() const;
  bool leftover_braces() const;

private:
  static QRegExp compile_regexp(const QString& key);
  void replace_all(const QRegExp& regexp, const value_variant& value);
  int replace_first(const QRegExp& regexp, const value_variant& value, int offset);
  QString m_string;
};

}  // namespace omm
