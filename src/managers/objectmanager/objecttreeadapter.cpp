#include "managers/objectmanager/objecttreeadapter.h"

#include <QItemSelection>
#include <glog/logging.h>

#include "objectmimedata.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "scene/project.h"
#include "common.h"
#include "commands/reparentobjectcommand.h"

namespace
{

bool can_move_drop_objects(const std::vector<omm::ObjectTreeContext>& contextes)
{
  const auto is_strictly_valid = [](const omm::ObjectTreeContext& context) {
    return context.is_strictly_valid();
  };

  const auto is_valid = [](const omm::ObjectTreeContext& context) {
    return context.is_valid();
  };

  // all contextes may be moved and at least one move is no noop
  return std::all_of(contextes.begin(), contextes.end(), is_valid)
      && std::any_of(contextes.begin(), contextes.end(), is_strictly_valid);
}

}  // namespace

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
  assert(&parent_item != nullptr);
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
  if (index.isValid()) {
    assert(index.internalPointer() != nullptr);
    return *static_cast<Object*>(index.internalPointer());
  } else {
    return m_root;
  }
}

QModelIndex ObjectTreeAdapter::index_of(Object& object) const
{
  if (object.is_root()) {
    return QModelIndex();
  } else {
    assert(&object != nullptr);
    return createIndex(object.row(), 0, &object);
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

void ObjectTreeAdapter::beginMoveObject(const ObjectTreeContext& context)
{
  assert(!context.subject.get().is_root());
  Object& old_parent = context.subject.get().parent();
  const auto old_pos = context.subject.get().row();
  const auto new_pos = context.get_insert_position();

  if (old_pos == new_pos) {
    m_last_move_was_noop = true;
  } else {
    beginMoveRows( index_of(old_parent), old_pos, old_pos,
                   index_of(context.parent), context.get_insert_position());
    m_last_move_was_noop = false;
  }
}

void ObjectTreeAdapter::endMoveObject()
{
  if (!m_last_move_was_noop) {
    endMoveRows();
  }
}

bool ObjectTreeAdapter::canDropMimeData( const QMimeData *data, Qt::DropAction action,
                                         int row, int column, const QModelIndex &parent ) const
{
  const auto new_contextes = make_new_contextes(data, row, parent);
  return data->hasFormat(ObjectMimeData::MIME_TYPE)
      && qobject_cast<const ObjectMimeData*>(data) != nullptr
      && can_move_drop_objects(new_contextes);
}

bool ObjectTreeAdapter::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent )
{
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  } else {
    const auto new_contextes = make_new_contextes(data, row, parent);
    Project& project = scene().project();
    project.submit(std::make_unique<ReparentObjectCommand>(project, new_contextes));
    return true;
  }
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
    const auto f = [this](const QModelIndex& index) {
      return ObjectRef(object_at(index));
    };
    const auto objects = ::transform<ObjectRef, std::vector>(indexes, f);
    return std::make_unique<ObjectMimeData>(objects).release();
  }
}

Scene& ObjectTreeAdapter::scene() const
{
  return m_root.scene();
}

std::vector<omm::ObjectTreeContext>
ObjectTreeAdapter::make_new_contextes( const QMimeData* data,
                                       int row, const QModelIndex& parent ) const
{
  std::vector<omm::ObjectTreeContext> new_contextes;

  auto object_mime_data = qobject_cast<const omm::ObjectMimeData*>(data);
  if (object_mime_data == nullptr) {
    return new_contextes;
  }

  omm::Object& new_parent = object_at(parent);
  const size_t pos = row < 0 ? new_parent.n_children() : row;

  new_contextes.reserve(object_mime_data->objects.size());
  const omm::Object* predecessor = (pos == 0) ? nullptr : &new_parent.child(pos - 1);
  for (omm::Object& subject : object_mime_data->objects) {
    new_contextes.emplace_back(subject, new_parent, predecessor);
    predecessor = &subject;
  }

  return new_contextes;
}

}  // namespace ommmake_new_contextes
