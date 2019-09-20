#pragma once

#include "managers/manager.h"
#include <memory>
#include "managers/dopesheet/dopesheet.h"

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

private:
  DopeSheetView* m_dope_sheet_view;
  std::unique_ptr<DopeSheet> m_dope_sheet;
};

}  // namespace omm
