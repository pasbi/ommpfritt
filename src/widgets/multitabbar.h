#pragma once

#include <QWidget>
#include <vector>
#include <set>
#include <QAbstractButton>
#include <memory>
#include <QBoxLayout>
#include <QTimer>

class QAbstractButton;

namespace omm
{

class MultiTabBar : public QWidget
{
  Q_OBJECT
public:
  explicit MultiTabBar();
  void add_tab(const QString& text);
  std::set<int> current_indices() const;
  void clear();

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

public Q_SLOTS:
  void set_current_indices(const std::set<int>& indices);

Q_SIGNALS:
  void current_indices_changed(const std::set<int>&);

private:
  std::vector<std::unique_ptr<QAbstractButton>> m_tabs;
  std::unique_ptr<QHBoxLayout> m_layout;
  QTimer m_change_current_on_drag_delay_timer;
  int m_drag_activated_index = -1;
  bool extend_selection() const;
};

}  // namespace omm
