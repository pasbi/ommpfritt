#pragma once

#include "commands/command.h"
#include "managers/quickaccessdelegate.h"
#include "objects/object.h"
#include "scene/history/macro.h"
#include <QAbstractItemDelegate>
#include <memory>

namespace omm
{
class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;
class Property;

class ObjectQuickAccessDelegate : public QuickAccessDelegate
{
public:
  explicit ObjectQuickAccessDelegate(QAbstractItemView& view);
};

}  // namespace omm
