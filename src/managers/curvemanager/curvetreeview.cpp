#include "managers/curvemanager/curvetreeview.h"

#include "animation/animator.h"
#include "animation/channelproxy.h"
#include "common.h"
#include "main/application.h"
#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include "managers/curvemanager/curvemanagerproxymodel.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <QHeaderView>
#include <QMouseEvent>
#include <QSortFilterProxyModel>

namespace
{

constexpr int QUICK_ACCESS_DELEGATE_COLUMN = 1;
constexpr int QUICK_ACCESS_DELEGATE_WIDTH = 20;

}  // namespace


namespace omm
{

CurveTreeView::CurveTreeView(Scene& scene)
    : m_scene(scene)
    , m_quick_access_delegate(
          std::make_unique<CurveManagerQuickAccessDelegate>(scene.animator(), *this))
    , m_proxy_model(std::make_unique<CurveManagerProxyModel>(scene.animator()))
    , m_expand_memory(*this, [this](const QModelIndex& index) { return map_to_animator(index); })
{
  connect(&scene.mail_box(),
          &MailBox::selection_changed,
          m_proxy_model.get(),
          &QSortFilterProxyModel::invalidate);

  setModel(m_proxy_model.get());

  setItemDelegateForColumn(QUICK_ACCESS_DELEGATE_COLUMN, m_quick_access_delegate.get());
  header()->setSectionResizeMode(QHeaderView::Fixed);
  header()->hide();

  connect(&scene.animator(),
          &Animator::modelReset,
          &m_expand_memory,
          &TreeExpandMemory::restore_later);
  connect(&scene.animator(),
          &Animator::rowsInserted,
          &m_expand_memory,
          &TreeExpandMemory::restore_later);
}

CurveTreeView::~CurveTreeView() = default;

CurveTreeView::Visibility CurveTreeView::is_visible(AbstractPropertyOwner& apo) const
{
  bool visible = false;
  bool invisible = false;
  Animator& animator = Application::instance().scene->animator();
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

CurveTreeView::Visibility CurveTreeView::is_visible(Property& property) const
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

CurveTreeView::Visibility CurveTreeView::is_visible(const std::pair<Property*, std::size_t>& channel) const
{
  auto it = m_channel_visible.find(channel);
  if (it == m_channel_visible.end()) {
    return Visibility::Visible;  // default
  } else {
    return it->second ? Visibility::Visible : Visibility::Hidden;
  }
}

CurveTreeView::Visibility CurveTreeView::is_visible(const ChannelProxy& channel) const
{
  return is_visible({&channel.track.property(), channel.channel});
}

void CurveTreeView::set_visible(AbstractPropertyOwner& apo, bool visible)
{
  {
    QSignalBlocker blocker(this);
    for (Property* property : m_scene.animator().accelerator().properties(apo)) {
      set_visible(*property, visible);
    }
  }
  notify_second_column_changed(m_scene.animator().index(apo));
}

void CurveTreeView::set_visible(Property& property, bool visible)
{
  {
    QSignalBlocker blocker(this);
    for (std::size_t c = 0; c < n_channels(property.variant_value()); ++c) {
      set_visible({&property, c}, visible);
    }
  }
  notify_second_column_changed(m_scene.animator().index(property));
}

void CurveTreeView::set_visible(const ChannelProxy& channel, bool visible)
{
  set_visible({&channel.track.property(), channel.channel}, visible);
}

void CurveTreeView::hide_everything()
{
  for (auto&& [k, v] : m_channel_visible) {
    v = false;
  }
  Q_EMIT m_proxy_model->dataChanged(QModelIndex(), QModelIndex());
  Q_EMIT visibility_changed();
}

QModelIndex CurveTreeView::map_to_animator(const QModelIndex& view_index) const
{
  return m_proxy_model->mapToSource(view_index);
}

QModelIndex CurveTreeView::map_from_animator(const QModelIndex& animator_index) const
{
  return m_proxy_model->mapFromSource(animator_index);
}

void CurveTreeView::set_visible(const std::pair<Property*, std::size_t>& channel, bool visible)
{
  m_channel_visible[channel] = visible;
  notify_second_column_changed(m_scene.animator().index(channel));
}

void CurveTreeView::resizeEvent(QResizeEvent* event)
{
  const int width = viewport()->width();
  static constexpr int gap = 6;
  header()->resizeSection(0, width - QUICK_ACCESS_DELEGATE_WIDTH - gap);
  header()->resizeSection(1, QUICK_ACCESS_DELEGATE_WIDTH);
  QTreeView::resizeEvent(event);
}

void CurveTreeView::mousePressEvent(QMouseEvent* event)
{
  m_mouse_down_index = indexAt(event->pos());
  if (m_mouse_down_index.column() == QUICK_ACCESS_DELEGATE_COLUMN) {
    m_quick_access_delegate->on_mouse_button_press(*event);
  } else {
    QTreeView::mousePressEvent(event);
  }
}

void CurveTreeView::mouseMoveEvent(QMouseEvent* event)
{
  if (m_mouse_down_index.column() == QUICK_ACCESS_DELEGATE_COLUMN) {
    m_quick_access_delegate->on_mouse_move(*event);
  } else {
    QTreeView::mouseMoveEvent(event);
  }
}

void CurveTreeView::mouseReleaseEvent(QMouseEvent* event)
{
  m_quick_access_delegate->on_mouse_release(*event);
  QTreeView::mouseReleaseEvent(event);
}

void CurveTreeView::notify_second_column_changed(const QModelIndex& sindex)
{
  QModelIndex index = map_from_animator(sindex).siblingAtColumn(QUICK_ACCESS_DELEGATE_COLUMN);
  while (index.isValid()) {
    Q_EMIT m_proxy_model->dataChanged(index, index);
    index = index.parent().siblingAtColumn(QUICK_ACCESS_DELEGATE_COLUMN);
  }
  Q_EMIT visibility_changed();
}

}  // namespace omm
