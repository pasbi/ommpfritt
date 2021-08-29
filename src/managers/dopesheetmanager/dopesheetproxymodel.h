#pragma once

#include <QIdentityProxyModel>
#include "animation/animator.h"
#include "proxychain.h"

namespace omm
{

class DopeSheetProxyModel : public QIdentityProxyModel
{
public:
  [[nodiscard]] int rowCount(const QModelIndex& index) const override;
  [[nodiscard]] bool hasChildren(const QModelIndex& index) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex& proxyIndex, int role) const override;

private:
  [[nodiscard]] omm::Animator* animator() const;
};

class ProxyModel : public ProxyChain
{
public:
  explicit ProxyModel();
};

}  // namespace omm
