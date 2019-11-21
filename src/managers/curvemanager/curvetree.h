#pragma once
#include <QTreeView>
#include "widgets/itemproxyview.h"
#include <memory>

namespace omm
{

class QuickAccessDelegate;
class Scene;

class CurveTree : public ItemProxyView<QTreeView>
{
public:
  explicit CurveTree(Scene& scene);
  static constexpr int quick_access_delegate_width = 20;

private:
  std::unique_ptr<QuickAccessDelegate> m_quick_access_delegate;
};


}  // namespace omm
