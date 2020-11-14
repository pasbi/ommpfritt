#pragma once

#include "managers/managertitlebar.h"
#include <QWidget>
#include <set>

class QPushButton;

namespace omm
{
class NodeManager;
class AbstractPropertyOwner;

class NodeManagerTitleBar : public ManagerTitleBar
{
  Q_OBJECT
public:
  explicit NodeManagerTitleBar(NodeManager& parent);
};

}  // namespace omm
