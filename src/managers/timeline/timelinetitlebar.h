#pragma once

#include <QWidget>
#include <memory>

namespace Ui { class TimeLineTitleBar; }
class QDockWidget;

namespace omm
{
class TimeLineTitleBar : public QWidget
{
  Q_OBJECT
public:
  explicit TimeLineTitleBar(QDockWidget& parent);
  ~TimeLineTitleBar();
  ::Ui::TimeLineTitleBar* ui() const { return m_ui.get(); }
  QSize sizeHint() const override { return QSize(0, 24); }

private:
  std::unique_ptr<::Ui::TimeLineTitleBar> m_ui;
};

}  // namespace omm
