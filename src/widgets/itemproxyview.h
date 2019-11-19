#pragma once

#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QAbstractItemView>
#include <QWidget>
#include <memory>
#include <KF5/KItemModels/KLinkItemSelectionModel>

namespace omm
{

class LinkItemSelectionModel : public KLinkItemSelectionModel
{
public:
  using KLinkItemSelectionModel::KLinkItemSelectionModel;
  void setCurrentIndex(const QModelIndex& index, QItemSelectionModel::SelectionFlags command) override;
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
  ItemProxyView(std::unique_ptr<QAbstractProxyModel> proxy, QWidget* parent = nullptr)
    : ViewT(parent)
  {
    set_proxy(std::move(proxy));
  }

  explicit ItemProxyView(QWidget* parent = nullptr) : ViewT(parent)
  {
    set_proxy(std::make_unique<QIdentityProxyModel>());
  }

  QAbstractItemModel* model() const { return m_proxy->sourceModel(); }
  QAbstractProxyModel* proxy() const { return m_proxy.get(); }

  void set_proxy(std::unique_ptr<QAbstractProxyModel> proxy)
  {
    assert(ViewT::model() == nullptr);  // only set the proxy if model has not yet been set.
    m_proxy = std::move(proxy);
  }

  void setModel(QAbstractItemModel* model) override
  {
    m_proxy->setSourceModel(model);
    m_selection_proxy.reset();
    ViewT::setModel(m_proxy.get());
  }

  void setSelectionModel(QItemSelectionModel* model) override
  {
    m_selection_proxy = std::make_unique<LinkItemSelectionModel>(m_proxy.get(), model);
    ViewT::setSelectionModel(m_selection_proxy.get());
  }

private:
  std::unique_ptr<QAbstractProxyModel> m_proxy;
  std::unique_ptr<LinkItemSelectionModel> m_selection_proxy;
};

}  // namespace omm
