#include "managers/curvemanager/curvetree.h"
#include "proxychain.h"
#include "common.h"
#include <QSortFilterProxyModel>
#include <KF5/KItemModels/kextracolumnsproxymodel.h>
#include "animation/animator.h"
#include "properties/property.h"
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include <QHeaderView>

namespace
{

class FilterSelectedProxyModel : public QSortFilterProxyModel
{
public:
  explicit FilterSelectedProxyModel()
  {
  }

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
  {
    return true;
    const auto* const animator = this->animator();
    if (animator == nullptr) {
      return true;
    } else if (!source_parent.isValid()) {
      const QModelIndex source_index = animator->index(source_row, 0, QModelIndex());
      assert(animator->index_type(source_index) == omm::Animator::IndexType::Owner);
      return ::contains(animator->scene.selection(), animator->owner(source_index));
    } else if (animator->index_type(source_parent) == omm::Animator::IndexType::Owner) {
      const QModelIndex source_index = animator->index(source_row, 0, source_parent);
      assert(animator->index_type(source_index) == omm::Animator::IndexType::Property);
      return omm::n_channels(animator->property(source_index)->variant_value()) > 0;
    } else {
      return true;
    }
  }

private:
  omm::Animator* animator() const { return static_cast<omm::Animator*>(sourceModel()); }
};

class AddColumnProxy : public KExtraColumnsProxyModel
{
public:
  explicit AddColumnProxy()
  {
    appendColumn();
  }

  QVariant
  extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const override
  {
    Q_UNUSED(parent)
    Q_UNUSED(row)
    Q_UNUSED(extraColumn)
    Q_UNUSED(role)
    // the extra column displays a delegate which does not rely on data.
    return QVariant();
  }
};

}  // namespace


namespace omm
{

CurveTree::CurveTree(Scene& scene)
  : m_quick_access_delegate(std::make_unique<CurveManagerQuickAccessDelegate>(*this))
{
  auto filter_proxy = std::make_unique<FilterSelectedProxyModel>();
  auto add_proxy = std::make_unique<AddColumnProxy>();
  connect(&scene.message_box(), SIGNAL(selection_changed(const std::set<AbstractPropertyOwner*>&)),
          filter_proxy.get(), SLOT(invalidate()));

  set_proxy(std::make_unique<ProxyChain>(ProxyChain::concatenate<std::unique_ptr<QAbstractProxyModel>>(
    std::move(filter_proxy),
    std::move(add_proxy)
  )));

  setModel(&scene.animator());
  setItemDelegateForColumn(1, m_quick_access_delegate.get());
  header()->setSectionResizeMode(1, QHeaderView::Fixed);
  header()->resizeSection(1, quick_access_delegate_width);
//  header()->hide();
}

}  // namespace omm
