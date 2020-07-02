#pragma once

#include <QObject>

namespace omm
{

class ModeSelector : public QObject
{
  Q_OBJECT

public:
  ModeSelector(const QString& cycle_action, const std::vector<QString>& activation_actions);
  virtual ~ModeSelector();

public:
  int mode() const { return m_mode; }

Q_SIGNALS:
  void mode_changed(int mode);

public Q_SLOTS:
  void set_mode(int mode);
  void on_action(const QString& action);
  void cycle();

private:
  int m_mode = 0;

public:
  const QString cycle_action;
  std::vector<QString> activation_actions;
};

}  // namespace omm
