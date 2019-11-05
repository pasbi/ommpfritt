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
  explicit ItemProxyView(QWidget* parent = nullptr)
    : ViewT(parent)
    , m_proxy(new QIdentityProxyModel)
  {
  }

  QAbstractItemModel* model() const { return m_proxy->sourceModel(); }
  QAbstractProxyModel* proxy() const { return m_proxy.get(); }

  void setModel(QAbstractItemModel* model) override
  {
    assert(!this->model());
    assert(!m_selection_proxy);
    m_proxy->setSourceModel(model);
    ViewT::setModel(m_proxy.get());
  }

  void setSelectionModel(QItemSelectionModel* selection_model) override
  {
    assert(this->model() != nullptr);             // set model first
    m_selection_proxy = std::make_unique<LinkItemSelectionModel>(m_proxy.get(), selection_model);
    ViewT::setSelectionModel(m_selection_proxy.get());
  }

  void set_proxy(std::unique_ptr<QAbstractProxyModel> proxy)
  {
    assert(this->model() == nullptr);             // set model first
    assert(this->selectionModel() == nullptr);    // set selection model first
    m_proxy = proxy;
  }

private:
  std::unique_ptr<QAbstractProxyModel> m_proxy;
  std::unique_ptr<LinkItemSelectionModel> m_selection_proxy;
};

}  // namespace omm
