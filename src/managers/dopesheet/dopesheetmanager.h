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
  std::string type() const override { return TYPE; }
  bool perform_action(const std::string &name) override;

private:
  DopeSheetView* m_dope_sheet_view;
};

}  // namespace omm
