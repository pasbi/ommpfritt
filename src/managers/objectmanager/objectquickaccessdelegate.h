#pragma once

#include <QAbstractItemDelegate>
#include "scene/history/macro.h"
#include <memory>
#include "objects/object.h"
#include "managers/quickaccessdelegate.h"
#include "commands/command.h"

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
