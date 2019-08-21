#include "scene/stylelist.h"
#include "scene/contextes.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"

namespace omm
{

StyleList::StyleList(Scene &scene) : scene(scene)
{

}

void StyleList::insert(ListOwningContext<Style> &context)
{
  const size_t row = this->insert_position(context.predecessor);
  beginInsertRows(QModelIndex(), row, row);
  List::insert(context);
  endInsertRows();
}

void StyleList::remove(ListOwningContext<Style> &context)
{
  const int row = position(context.subject);
  beginRemoveRows(QModelIndex(), row, row);
  List::remove(context);
  endRemoveRows();

}

std::unique_ptr<Style> StyleList::remove(Style &t)
{
  const int row = position(t);
  beginRemoveRows(QModelIndex(), row, row);
  auto removed_item = List::remove(t);
  endRemoveRows();
  return removed_item;
}

void StyleList::move(ListMoveContext<Style> &context)
{
  return List::move(context);
}

std::vector<std::unique_ptr<Style>> StyleList::set(std::vector<std::unique_ptr<Style>> items)
{
  beginResetModel();
  auto old_items = List::set(std::move(items));
  endResetModel();
  return old_items;
}

int StyleList::rowCount(const QModelIndex &parent) const
{
  assert(!parent.isValid());
  return List::items().size();
}

QVariant StyleList::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }
  assert(!index.parent().isValid());

  switch (role) {
  case Qt::DisplayRole:  [[fallthrough]];
  case Qt::EditRole:
    return QString::fromStdString(item(index.row()).name());
  case Qt::DecorationRole:
    return item(index.row()).icon();
  }
  return QVariant();
}

bool StyleList::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  assert(index.column() == 0);
  assert(!index.parent().isValid());

  switch (index.column()) {
  case 0:
  {
    Property* property = item_at(index).property(Object::NAME_PROPERTY_KEY);
    const auto svalue = value.toString().toStdString();
    if (property->value<std::string>() != svalue) {
      scene.submit<PropertiesCommand<StringProperty>>(std::set{ property }, svalue);
      return true;
    } else {
      return false;
    }
  }
  }
  return false;
}

Qt::ItemFlags StyleList::flags(const QModelIndex &index) const
{
  assert(!index.parent().isValid());
  return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
      | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren;
}

Style &StyleList::item_at(const QModelIndex &index) const
{
  assert(index.isValid());
  assert(!index.parent().isValid());
  return item(index.row());
}

QModelIndex StyleList::index_of(Style &style) const
{
  return createIndex(position(style), 0, &style);
}

}  // namespace omm
