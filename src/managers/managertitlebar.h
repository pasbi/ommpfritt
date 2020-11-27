#pragma once

#include <QWidget>
#include <memory>

class QHBoxLayout;
class QPushButton;

namespace omm
{
class Manager;

class ManagerTitleBar : public QWidget
{
  Q_OBJECT
public:
  explicit ManagerTitleBar(Manager& manager);
  [[nodiscard]] QSize sizeHint() const override;
  void add_widget(std::unique_ptr<QWidget> widget);

protected:
  [[nodiscard]] std::unique_ptr<QPushButton> make_lock_button() const;

private:
  QHBoxLayout* m_layout;
  Manager& m_manager;
};

}  // namespace omm
