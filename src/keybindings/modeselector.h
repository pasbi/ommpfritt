#pragma once

#include <QObject>

namespace omm
{

class CommandInterface;

class ModeSelector : public QObject
{
  Q_OBJECT

public:
  ModeSelector(CommandInterface& context, const QString& name, const QString& cycle_action,
               const std::vector<QString>& activation_actions);
  virtual ~ModeSelector();

public:
  int mode() const { return m_mode; }
  QString translated_name() const;
  bool handle(const QString& action_name);

Q_SIGNALS:
  void mode_changed(int mode);

public Q_SLOTS:
  void set_mode(int mode);
  void cycle();

private:
  int m_mode = 0;

public:
  const CommandInterface& context;
  const QString name;
  const QString cycle_action;
  const std::vector<QString> activation_actions;
};

}  // namespace omm
