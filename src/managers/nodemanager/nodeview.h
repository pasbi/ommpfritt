#pragma once

#include "cachedgetter.h"
#include "managers/panzoomcontroller.h"
#include "managers/range.h"
#include <QGraphicsItem>
#include <QGraphicsView>
#include <memory>

class QPainter;
class QMenu;
class QMimeData;

namespace omm
{

class NodeScene;
class PortItem;

namespace nodes
{
class AbstractPort;
class InputPort;
class Node;
class NodeModel;
}  // namespace nodes

class NodeView : public QGraphicsView
{
  Q_OBJECT

public:
  explicit NodeView(QWidget* parent = nullptr);
  ~NodeView() override;
  NodeView(NodeView&&) = delete;
  NodeView(const NodeView&) = delete;
  NodeView& operator=(NodeView&&) = delete;
  NodeView& operator=(const NodeView&) = delete;
  const QFont font;
  void abort();
  void remove_selection() const;
  void set_model(nodes::NodeModel* model);
  nodes::NodeModel* model() const;
  NodeScene* scene() const
  {
    return m_node_scene.get();
  }
  QPointF node_insert_pos() const
  {
    return m_node_insert_pos;
  }
  void populate_context_menu(QMenu& menu) const;
  void pan_to_center();
  bool accepts_paste(const QMimeData& mime_data) const;
  void reset_scene_rect();
  std::set<nodes::Node*> selected_nodes() const;

public:
  void copy_to_clipboard() const;
  void paste_from_clipboard();

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void drawForeground(QPainter* painter, const QRectF&) override;
  void drawBackground(QPainter* painter, const QRectF&) override;
  void draw_status_bar(QPainter& painter);
  void mousePressEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent*) override;
  void dragMoveEvent(QDragMoveEvent*) override;
  void dropEvent(QDropEvent*) override;

private:
  PanZoomController m_pan_zoom_controller;
  void draw_connection(QPainter& painter,
                       const QPointF& in,
                       const QPointF& out,
                       bool is_floating,
                       bool reverse) const;
  constexpr static auto m_droppable_kinds = Kind::Item;
  template<typename ItemT> ItemT* item_at(const QPoint& pos) const
  {
    if (QGraphicsItem* item = itemAt(pos); item != nullptr && item->type() == ItemT::TYPE) {
      return static_cast<ItemT*>(item);
    } else {
      return nullptr;
    }
  }

  bool can_drop(const QDropEvent& event);

  PortItem* m_tmp_connection_origin = nullptr;
  PortItem* m_tmp_connection_target = nullptr;
  nodes::InputPort* m_former_connection_target = nullptr;
  nodes::InputPort* m_about_to_disconnect = nullptr;
  bool m_aborted = false;
  QPoint m_last_mouse_position;
  QPointF m_node_insert_pos;
  std::unique_ptr<NodeScene> m_node_scene;
  QMetaObject::Connection m_view_scene_connection;
};

}  // namespace omm
