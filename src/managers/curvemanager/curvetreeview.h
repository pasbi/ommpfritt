#pragma once

#include "widgets/treeexpandmemory.h"
#include <QTreeView>
#include <memory>

class QSortFilterProxyModel;
class QAbstractProxyModel;

namespace omm
{
class QuickAccessDelegate;
class Scene;
class Track;
class Animator;
class AbstractPropertyOwner;
class Property;
class ChannelProxy;

class CurveTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit CurveTreeView(Scene& scene);
  ~CurveTreeView() override;
  CurveTreeView(CurveTreeView&&) = delete;
  CurveTreeView(const CurveTreeView&) = delete;
  CurveTreeView& operator=(CurveTreeView&&) = delete;
  CurveTreeView& operator=(const CurveTreeView&) = delete;
  enum class Visibility { Undetermined, Visible, Hidden };

  Visibility is_visible(AbstractPropertyOwner& apo) const;
  Visibility is_visible(Property& property) const;
  [[nodiscard]] Visibility is_visible(const std::pair<Property*, std::size_t>& channel) const;
  [[nodiscard]] Visibility is_visible(const ChannelProxy& channel) const;

  void set_visible(AbstractPropertyOwner& apo, bool visible);
  void set_visible(Property& property, bool visible);
  void set_visible(const std::pair<Property*, std::size_t>& channel, bool visible);
  void set_visible(const ChannelProxy& channel, bool visible);
  void hide_everything();

  QModelIndex map_to_animator(const QModelIndex& view_index) const;
  QModelIndex map_from_animator(const QModelIndex& animator_index) const;

protected:
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  std::map<AbstractPropertyOwner*, bool> m_apo_expanded;
  std::map<Property*, bool> m_property_expanded;
  std::map<std::pair<Property*, std::size_t>, bool> m_channel_visible;
  Scene& m_scene;
  std::unique_ptr<QuickAccessDelegate> m_quick_access_delegate;
  QModelIndex m_mouse_down_index;
  void notify_second_column_changed(const QModelIndex& sindex);
  std::unique_ptr<QSortFilterProxyModel> m_proxy_model;

  TreeExpandMemory m_expand_memory;

Q_SIGNALS:
  void visibility_changed();
};

}  // namespace omm
