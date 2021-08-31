#pragma once

#include "managers/quickaccessdelegate.h"

namespace omm
{
class Animator;
class CurveTreeView;

class CurveManagerQuickAccessDelegate : public QuickAccessDelegate
{
public:
  explicit CurveManagerQuickAccessDelegate(Animator& animator, CurveTreeView& view);
};

}  // namespace omm
