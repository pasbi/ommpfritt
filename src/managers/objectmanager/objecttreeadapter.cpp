#include "managers/objectmanager/objecttreeadapter.h"

#include <QItemSelection>
#include <glog/logging.h>

#include "objectmimedata.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "common.h"

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

bool ObjectTreeAdapter::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  switch (index.column()) {
  case 0:
    object_at(index).property(Object::NAME_PROPERTY_KEY).cast<std::string>().set_value(
      value.toString().toStdString()
    );
    return true;
  }

  return false;
}

QVariant ObjectTreeAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  switch (index.column()) {
  case 0:
    switch (role) {
    case Qt::DisplayRole:
      return QString::fromStdString(
        object_at(index).property(Object::NAME_PROPERTY_KEY).cast<std::string>().value()
      );
    }
  }
  return QVariant();
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
    return Qt::ItemIsDropEnabled;
  }

  switch (index.column()) {
  case 0:
    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  default:
    return QAbstractItemModel::flags(index);
  }
}

Qt::DropActions ObjectTreeAdapter::supportedDragActions() const
{
  return Qt::LinkAction | Qt::MoveAction | Qt::CopyAction;
}

void ObjectTreeAdapter::beginInsertObjects(Object& parent, int start, int end)
{
  beginInsertRows(index_of(parent), start, end);
}

void ObjectTreeAdapter::endInsertObjects()
{
  endInsertRows();
}

bool ObjectTreeAdapter::canDropMimeData( const QMimeData *data, Qt::DropAction action,
                                         int row, int column, const QModelIndex &parent ) const
{
  return data->hasFormat(ObjectMimeData::MIME_TYPE)
      && qobject_cast<const ObjectMimeData*>(data) != nullptr;
}

bool ObjectTreeAdapter::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent )
{
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  }

  auto object_mime_data = qobject_cast<const ObjectMimeData*>(data);
  if (object_mime_data == nullptr) {
    return false;
  }

  // TODO capsulate into undo/redo action
  const auto repudiate = [](Object& object) {
    assert(!object.is_root());
    return object.parent().repudiate(object);
  };

  auto dragged_objects = ::transform<std::unique_ptr<Object>>( object_mime_data->objects,
                                                               repudiate );
  Object& drop_target = object_at(parent);

  if (row == -1) {
    row = drop_target.n_children();
  }
  drop_target.adopt(std::move(dragged_objects), row);

  return true;
}

QStringList ObjectTreeAdapter::mimeTypes() const
{
  return { ObjectMimeData::MIME_TYPE };
}

QMimeData* ObjectTreeAdapter::mimeData(const QModelIndexList &indexes) const
{
  if (indexes.isEmpty()) {
    return nullptr;
  } else {
    using ObjectRef = std::reference_wrapper<Object>;
    const auto f = [this](const QModelIndex& index) {
      return ObjectRef(object_at(index));
    };

    const auto objects = ::transform<ObjectRef, std::vector>(indexes, f);
    return std::make_unique<ObjectMimeData>(objects).release();
  }
}

}  // namespace omm
