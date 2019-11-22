#include "proxychain.h"
#include "logging.h"
#include <QItemSelection>

namespace omm
{

ProxyChain::ProxyChain(std::vector<std::unique_ptr<QAbstractProxyModel>> proxies)
  : m_proxies(std::move(proxies))
{
}

ProxyChain::ProxyChain()
{
}

void ProxyChain::setSourceModel(QAbstractItemModel* source_model)
{
  for (auto it = m_proxies.begin(); it != m_proxies.end(); ++it) {
    (*it)->setSourceModel(source_model);
    source_model = it->get();
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

QModelIndex ProxyChain::mapFromChainSource(const QModelIndex &index) const
{
  assert(!index.isValid() || index.model() == m_proxies.front()->sourceModel());
  QModelIndex i = index;
  for (auto it = m_proxies.begin(); it != m_proxies.end(); ++it) {
    assert(i.model() == it->get()->sourceModel());
    i = (*it)->mapFromSource(i);
    assert(i.model() == it->get());
  }
  i = mapFromSource(i);
  assert(!i.isValid() || i.model() == this);
  return i;
}

QModelIndex ProxyChain::mapToChainSource(const QModelIndex& index) const
{
  assert(!index.isValid() || index.model() == this);
  QModelIndex i = index;
  for (auto it = m_proxies.rbegin(); it != m_proxies.rend(); ++it) {
    i = (*it)->mapToSource(i);
    assert(!i.isValid() || i.model() == (*it)->sourceModel());
  }
  assert(!i.isValid() || i.model() == m_proxies.front()->sourceModel());
  return i;
}

}  // namespace omm
