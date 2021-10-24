#pragma once

#include <QAbstractButton>
#include <QBoxLayout>
#include <QTimer>
#include <QWidget>
#include <memory>
#include <set>
#include <vector>

namespace omm
{
class MultiTabBar : public QWidget
{
  Q_OBJECT
public:
  explicit MultiTabBar();
  void add_tab(const QString& text);
  [[nodiscard]] std::set<int> current_indices() const;
  void clear();

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

public:
  void set_current_indices(const std::set<int>& indices);

Q_SIGNALS:
  void current_indices_changed(const std::set<int>&);

private:
  std::vector<std::unique_ptr<QAbstractButton>> m_tabs;
  std::unique_ptr<QHBoxLayout> m_layout;
  QTimer m_change_current_on_drag_delay_timer;
  int m_drag_activated_index = -1;
  [[nodiscard]] static bool extend_selection();
};

}  // namespace omm
