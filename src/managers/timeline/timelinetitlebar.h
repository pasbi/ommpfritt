#pragma once

#include "managers/managertitlebar.h"

namespace Ui
{
class TimeLineTitleBar;
}

namespace omm
{
class TimeLineTitleBar : public ManagerTitleBar
{
  Q_OBJECT
public:
  explicit TimeLineTitleBar(Manager& parent);
  ~TimeLineTitleBar() override;
  TimeLineTitleBar(TimeLineTitleBar&&) = delete;
  TimeLineTitleBar(const TimeLineTitleBar&) = delete;
  TimeLineTitleBar& operator=(TimeLineTitleBar&&) = delete;
  TimeLineTitleBar& operator=(const TimeLineTitleBar&) = delete;
  [[nodiscard]] ::Ui::TimeLineTitleBar* ui() const
  {
    return m_ui.get();
  }

private:
  std::unique_ptr<::Ui::TimeLineTitleBar> m_ui;
};

}  // namespace omm
