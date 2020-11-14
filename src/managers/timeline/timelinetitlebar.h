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
  ~TimeLineTitleBar();
  ::Ui::TimeLineTitleBar* ui() const
  {
    return m_ui.get();
  }

private:
  std::unique_ptr<::Ui::TimeLineTitleBar> m_ui;
};

}  // namespace omm
