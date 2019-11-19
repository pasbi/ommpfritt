#include "managers/curvemanager/curvemanager.h"
#include <QEvent>
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "managers/curvemanager/curvemanagertitlebar.h"
#include "managers/curvemanager/curvemanagerwidget.h"
#include <QSplitter>
#include "widgets/itemproxyview.h"
#include <QTreeView>
#include "animation/animator.h"
#include <QSortFilterProxyModel>

namespace
{

class ProxyModel : public QSortFilterProxyModel
{
public:
  explicit ProxyModel()
  {
  }

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
  {
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

}  // namespace

namespace omm
{

CurveManager::CurveManager(Scene& scene)
  : Manager(tr("Curves"), scene)
{
  setObjectName(TYPE);
  auto title_bar = std::make_unique<CurveManagerTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  auto curve_manager_widget = std::make_unique<CurveManagerWidget>(scene);
  m_widget = curve_manager_widget.get();

  auto proxy = std::make_unique<ProxyModel>();
  m_proxy = proxy.get();
  auto tree_widget = std::make_unique<ItemProxyView<QTreeView>>(std::move(proxy));
  tree_widget->setModel(&scene.animator());

  auto splitter = std::make_unique<QSplitter>(Qt::Horizontal);
  splitter->addWidget(tree_widget.release());
  splitter->addWidget(curve_manager_widget.release());

  set_widget(std::move(splitter));

  connect(&scene.message_box(), SIGNAL(selection_changed(const std::set<AbstractPropertyOwner*>&)),
          m_proxy, SLOT(invalidate()));
}

bool CurveManager::perform_action(const QString& name)
{
  return false;
}

}  // namespace omm
