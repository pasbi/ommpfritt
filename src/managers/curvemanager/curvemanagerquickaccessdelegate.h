#pragma once

#include "managers/quickaccessdelegate.h"

namespace omm
{

class CurveManagerQuickAccessDelegate : public QuickAccessDelegate
{
public:
  explicit CurveManagerQuickAccessDelegate(QAbstractItemView& view);
};

}  // namespace omm
