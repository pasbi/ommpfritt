#pragma once

#include <QToolBar>

namespace omm
{

class ToolBox;

class ToolBar : public QToolBar
{
  Q_OBJECT
public:
  explicit ToolBar(const QString& tools = "");
  QString tools() const { return m_tools; }
  void configure(const QString& tools);

  static constexpr auto TYPE = "ToolBar";
  virtual QString type() const { return TYPE; }

  static QStringList split(const QString& string);
  static constexpr auto separator = ';';
  static constexpr auto group_identifiers = std::pair { '[', ']' };
  static constexpr auto separator_identifier = "|";

private:
  QString m_tools;
};

}  // namespace omm
