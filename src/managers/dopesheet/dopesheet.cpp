#include "managers/dopesheet/dopesheet.h"
#include <QFormLayout>
#include <memory>
#include "animation/animator.h"
#include "scene/scene.h"
#include "animation/track.h"

namespace omm
{

DopeSheet::DopeSheet(Scene &scene) : m_scene(scene)
{

}

QModelIndex DopeSheet::index(int row, int column, const QModelIndex &parent) const
{
  return QModelIndex();
//  void* ptr = nullptr;
//  if (!parent.isValid()) {
//    ptr = m_scene.animator().get_owners()[row];
//  } else if (!parent.parent().isValid()) {
//    AbstractPropertyOwner* owner = m_scene.animator().get_owners()[parent.row()];
//    ptr = m_scene.animator().get_tracks(owner)[row];
//  }
//  return createIndex(row, column, ptr);
}

QModelIndex DopeSheet::parent(const QModelIndex &child) const
{
  return QModelIndex();
//  switch (index_type(child)) {
//  case IndexType::None:
//    [[fallthrough]];
//  case IndexType::Owner:
//    return QModelIndex();
//  case IndexType::Track:
//    return index(static_cast<Track*>(child.internalPointer())->owner());
//  default:
//    Q_UNREACHABLE();
//    return QModelIndex();
//  }
}

int DopeSheet::rowCount(const QModelIndex &parent) const
{
  return 0;
//  switch (index_type(parent)) {
//  case IndexType::None:
//    return m_scene.animator().get_owners().size();
//  case IndexType::Owner:
//    return m_scene.animator().get_tracks(owner(parent)).size();
//  case IndexType::Track:
//    [[fallthrough]];
//  default:
//    return 0;
//  }
}

int DopeSheet::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant DopeSheet::data(const QModelIndex &index, int role) const
{
  switch (role) {
  case Qt::EditRole:
    [[fallthrough]];
  case Qt::DisplayRole:
    switch (index_type(index)) {
    case IndexType::Owner:
      return QString::fromStdString(owner(index)->name());
    case IndexType::Track:
      return QString::fromStdString(track(index)->property().label());
    default:
      Q_UNREACHABLE();
    }
  }
  return QVariant();
}

QModelIndex DopeSheet::index(AbstractPropertyOwner *owner) const
{
  return QModelIndex();
//  const auto owners = m_scene.animator().get_owners();
//  const auto it = std::find(owners.begin(), owners.end(), owner);
//  assert(it != owners.end());
//  return createIndex(std::distance(owners.begin(), it), 0, *it);
}

DopeSheet::IndexType DopeSheet::index_type(const QModelIndex &index) const
{
  return IndexType::None;
//  if (index.isValid()) {
//    return IndexType::None;
//  }

//  const auto owners = m_scene.animator().get_owners();
//  const auto it = std::find(owners.begin(), owners.end(), index.internalPointer());
//  if (it != owners.end()) {
//    return IndexType::Owner;
//  } else {
//    return IndexType::Track;
//  }
}

Track *DopeSheet::track(const QModelIndex &index) const
{
  assert(index_type(index) == IndexType::Track);
  return static_cast<Track*>(index.internalPointer());
}

AbstractPropertyOwner* DopeSheet::owner(const QModelIndex &index) const
{
  assert(index_type(index) == IndexType::Owner);
  return static_cast<AbstractPropertyOwner*>(index.internalPointer());
}

}
