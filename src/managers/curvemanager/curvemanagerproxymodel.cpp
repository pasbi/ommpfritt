#include "managers/curvemanager/curvemanagerproxymodel.h"
#include "animation/animator.h"
#include "properties/property.h"
#include "scene/scene.h"

namespace omm
{

CurveManagerProxyModel::CurveManagerProxyModel(Animator& animator) : m_animator(animator)
{
  setSourceModel(&animator);
}

bool CurveManagerProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  using namespace omm;
  assert(!source_parent.isValid() || &m_animator == source_parent.model());
  const QModelIndex source_index = m_animator.index(source_row, 0, source_parent);
  switch (Animator::index_type(source_parent)) {
  case Animator::IndexType::None:
    assert(Animator::index_type(source_index) == Animator::IndexType::Owner);
    return ::contains(m_animator.scene.selection(), Animator::owner(source_index));
  case Animator::IndexType::Owner:
    assert(Animator::index_type(source_index) == Animator::IndexType::Property);
    return n_channels(Animator::property(source_index)->variant_value()) > 0;
  case Animator::IndexType::Property:
    assert(Animator::index_type(source_index) == Animator::IndexType::Channel);
    return sourceModel()->rowCount(source_parent) > 1;
  case Animator::IndexType::Channel:
    [[fallthrough]];
  default:
    Q_UNREACHABLE();
    return false;
  }
}

int CurveManagerProxyModel::columnCount(const QModelIndex&) const
{
  return Animator::COLUMN_COUNT + 1;
}

QVariant CurveManagerProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
  if (proxyIndex.column() == Animator::COLUMN_COUNT) {
    return QVariant();
  } else {
    return QSortFilterProxyModel::data(proxyIndex, role);
  }
}

}  // namespace omm
