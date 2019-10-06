#pragma once

#include <QWidget>
#include <memory>

class QHBoxLayout;

namespace omm
{

class Manager;

class ManagerTitleBar : public QWidget
{
  Q_OBJECT
public:
  explicit ManagerTitleBar(Manager& manager);
  QSize sizeHint() const override;
  void add_widget(std::unique_ptr<QWidget> widget);

private:
  QHBoxLayout* m_layout;
};

}  // namespace omm
