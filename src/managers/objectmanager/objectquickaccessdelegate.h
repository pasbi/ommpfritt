#pragma once

#include "managers/quickaccessdelegate.h"
#include <memory>

namespace omm
{

class ObjectQuickAccessDelegate : public QuickAccessDelegate
{
public:
  explicit ObjectQuickAccessDelegate(QAbstractItemView& view);
};

}  // namespace omm
