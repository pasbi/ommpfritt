#pragma once

#include "tools/selectpointsbasetool.h"

namespace omm
{

class TransformPointsHelper;

class SelectPointsTool : public SelectPointsBaseTool
{
public:
  using SelectPointsBaseTool::SelectPointsBaseTool;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");

};

}  // namespace omm
