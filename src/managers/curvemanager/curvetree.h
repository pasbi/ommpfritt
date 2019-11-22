#pragma once

#include <map>
#include <QTreeView>
#include "widgets/itemproxyview.h"
#include <memory>

namespace omm
{

class QuickAccessDelegate;
class Scene;
class Track;
class Animator;
class AbstractPropertyOwner;
class Property;
class ProxyChain;
class ChannelProxy;

class CurveTree : public ItemProxyView<QTreeView>
{
public:
  explicit CurveTree(Scene& scene);
  ~CurveTree();
  static constexpr int quick_access_delegate_width = 20;
  enum class Visibility { Undetermined, Visible, Hidden };

  Visibility is_visible(AbstractPropertyOwner& apo) const;
  Visibility is_visible(Property& property) const;
  Visibility is_visible(const std::pair<Property*, std::size_t>& channel) const;
  Visibility is_visible(const ChannelProxy& channel) const;

  void set_visible(AbstractPropertyOwner& apo, bool visible);
  void set_visible(Property& property, bool visible);
  void set_visible(const std::pair<Property*, std::size_t>& channel, bool visible);
  void set_visible(const ChannelProxy& channel, bool visible);

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
  void emit_data_changed_upwards(const QModelIndex& sindex);
};


}  // namespace omm
