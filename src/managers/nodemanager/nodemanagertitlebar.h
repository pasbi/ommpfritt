#pragma once

#include <set>
#include <QWidget>
#include "managers/managertitlebar.h"

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
