#include "managers/dopesheetmanager/dopesheetproxymodel.h"

namespace
{

bool is_property_item(const QModelIndex& index)
{
  return omm::Animator::index_type(index) == omm::Animator::IndexType::Property;
}

}  // namespace

namespace omm
{

ProxyModel::ProxyModel()
  : ProxyChain(ProxyChain::concatenate<std::unique_ptr<QAbstractProxyModel>>(
            std::make_unique<DopeSheetProxyModel>()))
{
}

int DopeSheetProxyModel::rowCount(const QModelIndex& index) const
{
  const auto* const animator = this->animator();
  if (animator == nullptr || is_property_item(mapToSource(index))) {
    return 0;
  } else {
    return QIdentityProxyModel::rowCount(index);
  }
}

int DopeSheetProxyModel::columnCount(const QModelIndex &parent) const
{
  const auto* const source = sourceModel();
  if (source == nullptr) {
    return 0;
  } else {
    assert(source->columnCount(parent) == Animator::COLUMN_COUNT);
    return Animator::COLUMN_COUNT + 1;
  }
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
  const auto* const animator = this->animator();
  if (animator == nullptr || is_property_item(mapToSource(index))) {
    return false;
  } else {
    return QIdentityProxyModel::hasChildren(index);
  }
}

Animator* DopeSheetProxyModel::animator() const
{
  return dynamic_cast<omm::Animator*>(sourceModel());
}

}  // namespace omm
