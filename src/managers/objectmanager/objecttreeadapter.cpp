#include "managers/objectmanager/objecttreeadapter.h"
#include <glog/logging.h>
#include "objects/object.h"
#include "scene/scene.h"



namespace omm
{

ObjectTreeAdapter::ObjectTreeAdapter(Object& root_object)
  : m_root(root_object)
{
}

ObjectTreeAdapter::~ObjectTreeAdapter()
{
}

QModelIndex ObjectTreeAdapter::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  const auto& parent_item = object_at(parent);
  return createIndex(row, column, &parent_item.child(row));
}

QModelIndex ObjectTreeAdapter::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  const Object& parent_item = object_at(index);
  if (parent_item.is_root()) {
    return QModelIndex();
  } else {
    return index_of(parent_item.parent());
  }
}

int ObjectTreeAdapter::rowCount(const QModelIndex& parent) const
{
  return object_at(parent).n_children();
}

int ObjectTreeAdapter::columnCount(const QModelIndex& parent) const
{
  return 3;
}

QVariant ObjectTreeAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  } else if (role != Qt::DisplayRole) {
    return QVariant();
  } else {
    return QString("abc %1 %2").arg(index.row()).arg(index.column());
  }
}

Object& ObjectTreeAdapter::object_at(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return m_root;
  } else {
    return *static_cast<Object*>(index.internalPointer());
  }
}

QModelIndex ObjectTreeAdapter::index_of(Object& object) const
{
  static const auto get_row = [](const Object& object) -> size_t
  {
    assert (!object.is_root());

    const std::vector<std::reference_wrapper<Object>> siblings = object.parent().children();
    for (size_t i = 0; i < siblings.size(); ++i) {
      if (&siblings[i].get() == &object) {
        return i;
      }
    }

    assert(false);
  };

  if (object.is_root()) {
    return QModelIndex();
  } else {
    return createIndex(get_row(object), 0, &object);
  }
}

QVariant ObjectTreeAdapter::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section) {
    case 0: return "object";
    case 1: return "is visible";
    case 2: return "tags";
    }
  }

 return QVariant();
}

Qt::ItemFlags ObjectTreeAdapter::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return 0;
  } else {
    return QAbstractItemModel::flags(index);
  }
}

void ObjectTreeAdapter::beginInsertObjects(Object& parent, int start, int end)
{
  beginInsertRows(index_of(parent), start, end);
}

void ObjectTreeAdapter::endInsertObjects()
{
  endInsertRows();
}


}  // namespace omm
