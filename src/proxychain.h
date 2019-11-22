#pragma once

#include <memory>
#include <QIdentityProxyModel>

namespace omm
{

// Model <--> Proxy 0 <--> Proxy 1 <--> ... <--> Proxy N <--> View
//            \___________________ ____________________/
//                                V
//                            ProxyChain

class ProxyChain : public QIdentityProxyModel
{
public:
  using Proxies = std::vector<std::unique_ptr<QAbstractProxyModel>>;
  explicit ProxyChain(Proxies proxies);
  ProxyChain();  // an empty proxy chain makes hardly sense. Consider to remove this.
  void setSourceModel(QAbstractItemModel* source_model) override;
  QModelIndex mapToChainSource(const QModelIndex& index) const;
  QModelIndex mapFromChainSource(const QModelIndex& index) const;

  template<typename T, typename... Ts>
  static std::vector<T> concatenate(Ts... ps)
  {
    std::vector<T> vs;
    vs.reserve(sizeof...(Ts));
    return concatenate_priv<T>(std::forward<std::vector<T>>(vs), std::forward<Ts>(ps)...);
  }

  template<typename T, typename... Ts>
  static std::vector<T> concatenate_priv(std::vector<T> vs, T p, Ts... ps)
  {
    vs.push_back(std::forward<T>(p));
    return concatenate_priv<T>(std::forward<std::vector<T>>(vs), std::forward<Ts>(ps)...);
  }

  template<typename T>
  static std::vector<T> concatenate_priv(std::vector<T> vs, T p)
  {
    vs.push_back(std::forward<T>(p));
    return vs;
  }

  QAbstractProxyModel& sub_proxy(std::size_t i) const;
  QAbstractItemModel* chainSourceModel() const;

private:
  std::vector<std::unique_ptr<QAbstractProxyModel>> m_proxies;
};

}
