#pragma once

#include "proxychain.h"
#include <KF5/KItemModels/KLinkItemSelectionModel>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QWidget>
#include <memory>

namespace omm
{
class LinkItemSelectionModel : public KLinkItemSelectionModel
{
public:
  using KLinkItemSelectionModel::KLinkItemSelectionModel;
  void setCurrentIndex(const QModelIndex& index,
                       QItemSelectionModel::SelectionFlags command) override;
};

/**
 * the ItemProxyView class encapsulates a View (must be derived from QAbstractItemView) with a
 * proxy model.
 * This solves the common problem that many views share one common model (and possibly selection
 * model), but each view should have its own proxy (i.e., for custom view-dependant filtering,
 * sorting or highlighting).
 */
template<typename ViewT> class ItemProxyView : public ViewT
{
  static_assert(std::is_base_of_v<QAbstractItemView, ViewT>);

public:
  ItemProxyView(std::unique_ptr<ProxyChain> proxy, QWidget* parent = nullptr) : ViewT(parent)
  {
    set_proxy(std::move(proxy));
  }

  explicit ItemProxyView(QWidget* parent = nullptr) : ViewT(parent)
  {
    set_proxy(std::make_unique<ProxyChain>());
  }

  [[nodiscard]] ProxyChain* model() const
  {
    return m_proxy.get();
  }

  void set_proxy(std::unique_ptr<ProxyChain> proxy)
  {
    if (m_proxy) {
      QAbstractItemModel* model = m_proxy->chainSourceModel();
      m_proxy = std::move(proxy);
      ItemProxyView::setModel(model);
    } else {
      m_proxy = std::move(proxy);
    }
  }

  void setModel(QAbstractItemModel* model) override
  {
    m_proxy->setSourceModel(model);
    m_selection_proxy.reset();
    ViewT::setModel(m_proxy.get());
    ItemProxyView::setSelectionModel(new QItemSelectionModel(model, this));
  }

  void setSelectionModel(QItemSelectionModel* model) override
  {
    m_selection_proxy = std::make_unique<LinkItemSelectionModel>(m_proxy.get(), model);
    ViewT::setSelectionModel(m_selection_proxy.get());
  }

private:
  std::unique_ptr<ProxyChain> m_proxy;
  std::unique_ptr<LinkItemSelectionModel> m_selection_proxy;
};

}  // namespace omm
