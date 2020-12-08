#include "proxychain.h"
#include "logging.h"
#include <QItemSelection>

namespace omm
{
ProxyChain::ProxyChain(std::vector<std::unique_ptr<QAbstractProxyModel>> proxies)
    : m_proxies(std::move(proxies))
{
}

ProxyChain::ProxyChain() = default;

void ProxyChain::setSourceModel(QAbstractItemModel* source_model)
{
  for (auto& m_proxie : m_proxies) {
    m_proxie->setSourceModel(source_model);
    source_model = m_proxie.get();
  }
  QIdentityProxyModel::setSourceModel(source_model);
}

QAbstractProxyModel& ProxyChain::sub_proxy(std::size_t i) const
{
  return *m_proxies.at(i);
}

QAbstractItemModel* ProxyChain::chainSourceModel() const
{
  if (m_proxies.empty()) {
    return sourceModel();
  } else {
    return m_proxies.front()->sourceModel();
  }
}

QModelIndex ProxyChain::mapFromChainSource(const QModelIndex& index) const
{
  assert(!index.isValid() || index.model() == chainSourceModel());
  QModelIndex i = index;
  for (const auto& proxy : m_proxies) {
    assert(!i.isValid() || i.model() == proxy->sourceModel());
    i = proxy->mapFromSource(i);
    assert(!i.isValid() || i.model() == proxy.get());
  }
  i = mapFromSource(i);
  assert(!i.isValid() || i.model() == this);
  return i;
}

QModelIndex ProxyChain::mapToChainSource(const QModelIndex& index) const
{
  assert(!index.isValid() || index.model() == this);
  QModelIndex i = mapToSource(index);
  for (auto it = m_proxies.rbegin(); it != m_proxies.rend(); ++it) {
    assert(!i.isValid() || i.model() == it->get());
    i = (*it)->mapToSource(i);
    assert(!i.isValid() || i.model() == (*it)->sourceModel());
  }
  assert(!i.isValid() || i.model() == chainSourceModel());
  return i;
}

}  // namespace omm
