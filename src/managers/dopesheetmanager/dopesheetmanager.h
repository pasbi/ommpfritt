#pragma once

#include "managers/manager.h"
#include <memory>

namespace omm
{
class DopeSheet;
class DopeSheetView;

class DopeSheetManager : public Manager
{
  Q_OBJECT
public:
  explicit DopeSheetManager(Scene& scene);
  static constexpr auto TYPE = "DopeSheetManager";
  [[nodiscard]] QString type() const override
  {
    return TYPE;
  }
  bool perform_action(const QString& name) override;

private:
  DopeSheetView* m_dope_sheet_view;
};

}  // namespace omm
