#include "proxychain.h"
#include "logging.h"
#include <QItemSelection>

namespace omm
{

ProxyChain::ProxyChain(std::vector<std::unique_ptr<QAbstractProxyModel>> proxies)
  : m_proxies(std::move(proxies))
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

}  // namespace omm
