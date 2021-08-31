#include "managers/dopesheetmanager/dopesheetproxymodel.h"
#include "animation/animator.h"

namespace
{

bool is_property_item(const QModelIndex& index)
{
  return omm::Animator::index_type(index) == omm::Animator::IndexType::Property;
}

}  // namespace

namespace omm
{

DopeSheetProxyModel::DopeSheetProxyModel(Animator& animator) : m_animator(animator)
{
  QIdentityProxyModel::setSourceModel(&animator);
}

int DopeSheetProxyModel::rowCount(const QModelIndex& index) const
{
  if (is_property_item(mapToSource(index))) {
    return 0;
  } else {
    return QIdentityProxyModel::rowCount(index);
  }
}

int DopeSheetProxyModel::columnCount(const QModelIndex&) const
{
  return Animator::COLUMN_COUNT + 1;
}

QVariant DopeSheetProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
  if (proxyIndex.column() == Animator::COLUMN_COUNT) {
    return QVariant();
  } else {
    return QIdentityProxyModel::data(proxyIndex, role);
  }
}

bool DopeSheetProxyModel::hasChildren(const QModelIndex& index) const
{
  if (is_property_item(mapToSource(index))) {
    return false;
  } else {
    return QIdentityProxyModel::hasChildren(index);
  }
}

Animator& DopeSheetProxyModel::animator() const
{
  return m_animator;
}

}  // namespace omm
