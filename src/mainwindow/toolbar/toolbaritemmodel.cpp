#include "mainwindow/toolbar/toolbaritemmodel.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include "mainwindow/toolbar/toolbar.h"
#include <QMimeData>
#include <QTreeWidgetItem>
#include "mainwindow/application.h"

namespace
{

class Item : public QStandardItem
{
protected:
  explicit Item() : QStandardItem(0, 1)
  {
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  }
public:
  virtual QString encode() const = 0;
};

class ActionItem : public Item
{
public:
  static constexpr int TYPE = QStandardItem::UserType + 1;
  explicit ActionItem(const QString& code) : command_name(code)
  {
    const auto& key_bindings = omm::Application::instance().key_bindings;
    const auto* item = key_bindings.value(omm::Application::TYPE, command_name);
    setData(item->translated_name(omm::KeyBindings::TRANSLATION_CONTEXT), Qt::DisplayRole);
    setData(item->icon(), Qt::DecorationRole);
  }

  int type() const override { return TYPE; }
  QString encode() const override
  {
    return command_name;
  }
  const QString command_name;
};

class GroupItem : public Item
{
public:
  static constexpr int TYPE = QStandardItem::UserType + 2;
  explicit GroupItem()
  {
    setData(omm::ToolBarItemModel::tr("group"), Qt::DisplayRole);
    setFlags(flags() | Qt::ItemIsDropEnabled);
  }

  QString encode() const override
  {
    QStringList items;
    for (int i = 0; i < rowCount(); ++i) {
      items.push_back(static_cast<const Item*>(child(i, 0))->encode());
    }
    return QString("[%1]").arg(items.join(omm::ToolBar::separator));
  }

  int type() const override { return TYPE; }
};

class SeparatorItem : public Item
{
public:
  static constexpr int TYPE = QStandardItem::UserType + 3;
  explicit SeparatorItem()
  {
    setData(omm::ToolBarItemModel::tr("separator"), Qt::DisplayRole);
  }

  QString encode() const override
  {
    return omm::ToolBar::separator_identifier;
  }

  int type() const override { return TYPE; }
};

}  // namespace

namespace omm
{

ToolBarItemModel::ToolBarItemModel(const QString& code)
{
  decode(code, 0, QModelIndex());
}

QString ToolBarItemModel::encode(int row_begin, int row_end, const QModelIndex& parent) const
{
  QStringList items;
  for (int row = row_begin; row < row_end; ++row) {
    const Item* item = static_cast<const Item*>(itemFromIndex(index(row, 0, parent)));
    items.push_back(item->encode());
  }
  return items.join(ToolBar::separator);
}

QString ToolBarItemModel::encode() const
{
  return encode(0, rowCount(), QModelIndex());
}

void ToolBarItemModel::add_button()
{
  const int row = rowCount();
  beginInsertRows(QModelIndex(), row, row);
  insertRow(row, new GroupItem());
  endInsertRows();
}

void ToolBarItemModel::add_separator()
{
  const int row = rowCount();
  beginInsertRows(QModelIndex(), row, row);
  insertRow(row, new SeparatorItem());
  endInsertRows();
}

void ToolBarItemModel::remove_selection(const QItemSelection& selection)
{
  auto sorted_selection = selection;
  std::sort(sorted_selection.begin(), sorted_selection.end(),
            [](const QItemSelectionRange& a, const QItemSelectionRange& b)
  {
    if (a.top() < b.top()) {
      assert(a.bottom() < b.top());  // no overlap
      return false;
    } else {
      assert(a.top() > b.bottom());  // no overlap
      return true;
    }
  });
  for (const auto& range : sorted_selection) {
    beginRemoveRows(range.parent(), range.top(), range.bottom());
    removeRows(range.top(), range.height(), range.parent());
    endRemoveRows();
  }
}

void ToolBarItemModel::decode(const QString& code, int row, const QModelIndex& parent)
{
  QList<QStandardItem*> items;
  std::map<GroupItem*, QString> groups;
  for (QString item : ToolBar::split(code)) {
    if (item.startsWith(ToolBar::group_identifiers.first)) {
      assert(item.endsWith(ToolBar::group_identifiers.second));
      auto group_item = std::make_unique<GroupItem>();
      groups[group_item.get()] = item.mid(1, item.size() - 2);
      items.push_back(group_item.release());
    } else if (item == ToolBar::separator_identifier) {
      items.push_back(std::make_unique<SeparatorItem>().release());
    } else {
      items.push_back(std::make_unique<ActionItem>(item).release());
    }
  }

  row = row < 0 ? rowCount(parent) : row;
  beginInsertRows(parent, row, row + items.size() - 1);
  if (parent.isValid()) {
    itemFromIndex(parent)->insertRows(row, items);
  } else {
    for (int i = 0; i < items.size(); ++i) {
      insertRow(row + i, items[i]);
    }
  }
  endInsertRows();

  for (const auto& [group_item, actions] : groups) {
    const QModelIndex index = indexFromItem(group_item);
    assert(index.isValid());
    decode(actions, 0, index);
  }
}

bool ToolBarItemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                       int row, int column, const QModelIndex& parent) const
{
  Q_UNUSED(column)
  Q_UNUSED(row)
  Q_UNUSED(parent)
  return data->hasFormat(ToolBarDialog::mime_type) && ::contains(std::set{ Qt::MoveAction, Qt::LinkAction}, action);
}

bool ToolBarItemModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                    int row, int column, const QModelIndex& parent)
{
  assert(canDropMimeData(data, action, row, column, parent));
  QDataStream stream(data->data(ToolBarDialog::mime_type));
  QString code;
  stream >> code;
  decode(code, row, parent);
  return false;
}

QMimeData* ToolBarItemModel::mimeData(const QModelIndexList& indices) const
{
  QByteArray data;

  {
    QDataStream stream(&data, QIODevice::WriteOnly);

    const QModelIndexList filtered_indices = ::filter_if(indices, [](const QModelIndex& index)
    {
      return index.column() == 0;
    });

    QStringList items;
    for (const QModelIndex& index : filtered_indices) {
      if (index.isValid() && index.column() == 0) {
        items.push_back(static_cast<const Item*>(itemFromIndex(index))->encode());
      }
    }
    stream << items.join(ToolBar::separator);
  }

  auto mime_data = std::make_unique<QMimeData>();
  mime_data->setData(omm::ToolBarDialog::mime_type, data);
  return mime_data.release();
}



}  // namespace omm
