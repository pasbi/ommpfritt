#include "managers/curvemanager/curvetree.h"
#include "animation/animator.h"
#include "animation/channelproxy.h"
#include "common.h"
#include "mainwindow/application.h"
#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include "properties/property.h"
#include "proxychain.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <KF5/KItemModels/kextracolumnsproxymodel.h>
#include <QHeaderView>
#include <QMouseEvent>
#include <QSortFilterProxyModel>

namespace
{
class FilterSelectedProxyModel : public QSortFilterProxyModel
{
public:
  explicit FilterSelectedProxyModel(omm::Animator& animator) : m_animator(animator)
  {
  }

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
  {
    assert(!source_parent.isValid() || &m_animator == source_parent.model());
    const QModelIndex source_index = m_animator.index(source_row, 0, source_parent);
    switch (m_animator.index_type(source_parent)) {
    case omm::Animator::IndexType::None:
      assert(m_animator.index_type(source_index) == omm::Animator::IndexType::Owner);
      return ::contains(m_animator.scene.selection(), m_animator.owner(source_index));
    case omm::Animator::IndexType::Owner:
      assert(m_animator.index_type(source_index) == omm::Animator::IndexType::Property);
      return omm::n_channels(m_animator.property(source_index)->variant_value()) > 0;
    case omm::Animator::IndexType::Property:
      assert(m_animator.index_type(source_index) == omm::Animator::IndexType::Channel);
      return sourceModel()->rowCount(source_parent) > 1;
    case omm::Animator::IndexType::Channel:
      [[fallthrough]];
    default:
      Q_UNREACHABLE();
      return false;
    }
  }

private:
  omm::Animator& m_animator;
};

class AddColumnProxy : public QIdentityProxyModel
{
public:
  explicit AddColumnProxy()
  {
  }

  QModelIndex index(int row, int column, const QModelIndex& parent) const override
  {
    const auto index = QIdentityProxyModel::index(row, 0, parent);
    return createIndex(row, column, index.internalPointer());
  }

  int columnCount(const QModelIndex& index) const override
  {
    return sourceModel()->columnCount(mapToSource(index)) + 1;
  }

  QVariant data(const QModelIndex& index, int role) const override
  {
    if (index.column() < sourceModel()->columnCount(mapToSource(index))) {
      return mapToSource(index).data(role);
    } else {
      // the extra column displays a delegate which does not rely on data.
      return QVariant();
    }
  }
};

}  // namespace

namespace omm
{
CurveTree::CurveTree(Scene& scene)
    : m_scene(scene),
      m_quick_access_delegate(
          std::make_unique<CurveManagerQuickAccessDelegate>(scene.animator(), *this))
{
  auto filter_proxy = std::make_unique<FilterSelectedProxyModel>(scene.animator());
  auto add_proxy = std::make_unique<AddColumnProxy>();
  connect(&scene.mail_box(),
          qOverload<const std::set<AbstractPropertyOwner*>&>(&MailBox::selection_changed),
          filter_proxy.get(),
          &QSortFilterProxyModel::invalidate);

  m_add_column_proxy = add_proxy.get();

  set_proxy(std::make_unique<ProxyChain>(
      ProxyChain::concatenate<std::unique_ptr<QAbstractProxyModel>>(std::move(filter_proxy),
                                                                    std::move(add_proxy))));

  setModel(&scene.animator());
  setItemDelegateForColumn(m_quick_access_delegate_column, m_quick_access_delegate.get());
  header()->setSectionResizeMode(QHeaderView::Fixed);
  header()->setStretchLastSection(true);
  header()->hide();
}

CurveTree::~CurveTree()
{
}

CurveTree::Visibility CurveTree::is_visible(AbstractPropertyOwner& apo) const
{
  bool visible = false;
  bool invisible = false;
  Animator& animator = Application::instance().scene.animator();
  for (Property* property : animator.accelerator().properties(apo)) {
    if (n_channels(property->variant_value()) > 0) {
      switch (is_visible(*property)) {
      case Visibility::Hidden:
        invisible = true;
        break;
      case Visibility::Visible:
        visible = true;
        break;
      case Visibility::Undetermined:
        return Visibility::Undetermined;
      }

      if (visible && invisible) {
        return Visibility::Undetermined;
      }
    }
  }
  if (visible) {
    return Visibility::Visible;
  } else {
    return Visibility::Hidden;
  }
}

CurveTree::Visibility CurveTree::is_visible(Property& property) const
{
  bool visible = false;
  bool invisible = false;
  const std::size_t n = n_channels(property.variant_value());
  assert(n > 0);
  for (std::size_t c = 0; c < n; ++c) {
    switch (is_visible({&property, c})) {
    case Visibility::Visible:
      visible = true;
      break;
    case Visibility::Hidden:
      invisible = true;
      break;
    default:
      Q_UNREACHABLE();
    }

    if (visible && invisible) {
      return Visibility::Undetermined;
    }
  }
  if (visible) {
    return Visibility::Visible;
  } else if (invisible) {
    return Visibility::Hidden;
  } else {
    Q_UNREACHABLE();
    return Visibility::Undetermined;
  }
}

CurveTree::Visibility CurveTree::is_visible(const std::pair<Property*, std::size_t>& channel) const
{
  auto it = m_channel_visible.find(channel);
  if (it == m_channel_visible.end()) {
    return Visibility::Visible;  // default
  } else {
    return it->second ? Visibility::Visible : Visibility::Hidden;
  }
}

CurveTree::Visibility CurveTree::is_visible(const ChannelProxy& channel) const
{
  return is_visible({&channel.track.property(), channel.channel});
}

void CurveTree::set_visible(AbstractPropertyOwner& apo, bool visible)
{
  {
    QSignalBlocker blocker(this);
    for (Property* property : m_scene.animator().accelerator().properties(apo)) {
      set_visible(*property, visible);
    }
  }
  notify_second_column_changed(m_scene.animator().index(apo));
}

void CurveTree::set_visible(Property& property, bool visible)
{
  {
    QSignalBlocker blocker(this);
    for (std::size_t c = 0; c < n_channels(property.variant_value()); ++c) {
      set_visible({&property, c}, visible);
    }
  }
  notify_second_column_changed(m_scene.animator().index(property));
}

void CurveTree::set_visible(const ChannelProxy& channel, bool visible)
{
  set_visible({&channel.track.property(), channel.channel}, visible);
}

void CurveTree::hide_everything()
{
  for (auto&& [k, v] : m_channel_visible) {
    v = false;
  }
  Q_EMIT m_add_column_proxy->dataChanged(QModelIndex(), QModelIndex());
  Q_EMIT visibility_changed();
}

void CurveTree::set_visible(const std::pair<Property*, std::size_t>& channel, bool visible)
{
  m_channel_visible[channel] = visible;
  notify_second_column_changed(m_scene.animator().index(channel));
}

void CurveTree::resizeEvent(QResizeEvent* event)
{
  const int width = viewport()->width();
  header()->resizeSection(0, width - quick_access_delegate_width);
  ItemProxyView::resizeEvent(event);
}

void CurveTree::mousePressEvent(QMouseEvent* event)
{
  m_mouse_down_index = indexAt(event->pos());
  if (m_mouse_down_index.column() == m_quick_access_delegate_column) {
    m_quick_access_delegate->on_mouse_button_press(*event);
  } else {
    ItemProxyView::mousePressEvent(event);
  }
}

void CurveTree::mouseMoveEvent(QMouseEvent* event)
{
  if (m_mouse_down_index.column() == m_quick_access_delegate_column) {
    m_quick_access_delegate->on_mouse_move(*event);
  } else {
    ItemProxyView::mouseMoveEvent(event);
  }
}

void CurveTree::mouseReleaseEvent(QMouseEvent* event)
{
  m_quick_access_delegate->on_mouse_release(*event);
  ItemProxyView::mouseReleaseEvent(event);
}

void CurveTree::notify_second_column_changed(const QModelIndex& sindex)
{
  ProxyChain& proxy_chain = static_cast<ProxyChain&>(*ItemProxyView::model());
  QModelIndex index
      = proxy_chain.mapFromChainSource(sindex).siblingAtColumn(m_quick_access_delegate_column);
  while (index.isValid()) {
    Q_EMIT m_add_column_proxy->dataChanged(index, index);
    index = index.parent().siblingAtColumn(m_quick_access_delegate_column);
  }
  Q_EMIT visibility_changed();
}

}  // namespace omm
