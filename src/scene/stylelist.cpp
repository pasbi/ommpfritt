#include "scene/stylelist.h"
#include "commands/propertycommand.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include "properties/stringproperty.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include "scene/mailbox.h"
#include "scene/scene.h"

namespace omm
{
StyleList::StyleList(Scene& scene)
    : ItemModelAdapter<StyleList, Style, QAbstractListModel>(scene, *this), scene(scene)
{
  connect(&scene.mail_box(), &MailBox::style_appearance_changed, this, [this](Style& style) {
    if (this->contains(style)) {
      const QModelIndex index = index_of(style);
      Q_EMIT dataChanged(index, index, {Qt::DecorationRole});
    }
  });
}

void StyleList::insert(ListOwningContext<Style>& context)
{
  const std::size_t row = this->insert_position(context.predecessor);
  beginInsertRows(QModelIndex(), row, row);
  List::insert(context);
  endInsertRows();
  Q_EMIT scene.mail_box().style_inserted(context.get_subject());
}

void StyleList::remove(ListOwningContext<Style>& context)
{
  const int row = position(context.subject);
  beginRemoveRows(QModelIndex(), row, row);
  List::remove(context);
  endRemoveRows();
  Q_EMIT scene.mail_box().style_removed(context.get_subject());
}

std::unique_ptr<Style> StyleList::remove(Style& t)
{
  const int row = position(t);
  beginRemoveRows(QModelIndex(), row, row);
  auto removed_item = List::remove(t);
  endRemoveRows();
  Q_EMIT scene.mail_box().style_removed(t);
  return removed_item;
}

void StyleList::move(ListMoveContext<Style>& context)
{
  List::move(context);
  Q_EMIT scene.mail_box().style_moved(context.get_subject());
}

std::deque<std::unique_ptr<Style> > StyleList::set(std::deque<std::unique_ptr<Style> > items)
{
  beginResetModel();
  auto old_items = List::set(std::move(items));
  endResetModel();
  return old_items;
}

int StyleList::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  assert(!parent.isValid());
  return List::items().size();
}

QVariant StyleList::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }
  assert(!index.parent().isValid());

  switch (role) {
  case Qt::DisplayRole:
    [[fallthrough]];
  case Qt::EditRole:
    return item(index.row()).name();
  case Qt::DecorationRole:
    return IconProvider::icon(item(index.row()));
    ;
  }
  return QVariant();
}

bool StyleList::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  assert(index.column() == 0);
  assert(!index.parent().isValid());

  switch (index.column()) {
  case 0: {
    Property* property = item_at(index).property(Object::NAME_PROPERTY_KEY);
    const auto svalue = value.toString();
    if (property->value<QString>() != svalue) {
      scene.submit<PropertiesCommand<StringProperty>>(std::set{property}, svalue);
      return true;
    } else {
      return false;
    }
  }
  }
  return false;
}

Qt::ItemFlags StyleList::flags(const QModelIndex& index) const
{
  Q_UNUSED(index)
  assert(!index.parent().isValid());
  return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren;
}

Style& StyleList::item_at(const QModelIndex& index) const
{
  assert(index.isValid());
  assert(!index.parent().isValid());
  return item(index.row());
}

QModelIndex StyleList::index_of(Style& style) const
{
  return createIndex(position(style), 0, &style);
}

}  // namespace omm
