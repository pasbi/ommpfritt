#pragma once

#include <QTreeView>
#include <map>
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

class CurveTree : public QTreeView
{
  Q_OBJECT
public:
  explicit CurveTree(Scene& scene);
  ~CurveTree() override;
  CurveTree(CurveTree&&) = delete;
  CurveTree(const CurveTree&) = delete;
  CurveTree& operator=(CurveTree&&) = delete;
  CurveTree& operator=(const CurveTree&) = delete;
  static constexpr int quick_access_delegate_width = 20;
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

  QModelIndex map_to_source(const QModelIndex& view_index) const;
  QModelIndex map_from_source(const QModelIndex& animator_index) const;

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
  static constexpr int m_quick_access_delegate_column = 1;
  std::unique_ptr<QuickAccessDelegate> m_quick_access_delegate;
  QModelIndex m_mouse_down_index;
  void notify_second_column_changed(const QModelIndex& sindex);
  std::unique_ptr<QSortFilterProxyModel> m_sort_filter_proxy;
  std::unique_ptr<QAbstractProxyModel> m_add_column_proxy;

  std::map<const void*, bool> m_expanded_state;
  void restore_expanded_state_later();

Q_SIGNALS:
  void visibility_changed();
};

}  // namespace omm
