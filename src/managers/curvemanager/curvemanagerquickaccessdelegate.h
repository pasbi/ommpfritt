#pragma once

#include "managers/quickaccessdelegate.h"

namespace omm
{

class Animator;
class CurveTree;

class CurveManagerQuickAccessDelegate : public QuickAccessDelegate
{
public:
  explicit CurveManagerQuickAccessDelegate(Animator& animator, CurveTree& view);
};

}  // namespace omm
