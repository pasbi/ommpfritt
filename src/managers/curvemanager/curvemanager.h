#pragma once

#include "managers/manager.h"
#include "managers/timeline/timelinecanvas.h"

namespace omm
{

class AbstractPropertyOwner;
class CurveManagerTitleBar;
class CurveManagerWidget;

class CurveManager : public Manager
{
  Q_OBJECT
public:
  explicit CurveManager(Scene& scene);
  static constexpr auto TYPE = "CurveManager";
  QString type() const override { return TYPE; }
  bool perform_action(const QString& name) override;

private:

public Q_SLOTS:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void set_locked(bool locked) { m_is_locked = locked; }

private:
  bool m_is_locked = false;
  CurveManagerTitleBar* m_title_bar;
  CurveManagerWidget* m_widget;
};

}  // namespace omm
