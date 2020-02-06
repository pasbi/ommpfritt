#pragma once

#include "aspects/abstractpropertyowner.h"
#include "managers/panzoomcontroller.h"
#include "cachedgetter.h"
#include <QWidget>
#include "managers/range.h"

class QPainter;
class QMenu;
class QMimeData;

namespace omm
{

class NodeModel;
class InputPort;
class AbstractPort;
class Node;

class NodeView : public QWidget
{
  Q_OBJECT

public:
  explicit NodeView(QWidget* parent = nullptr);
  ~NodeView();
  void set_model(NodeModel* model);
  NodeModel* model() const { return m_model; }
  const QFont font;
  void abort();
  void remove_selection();
  std::set<Node*> selected_nodes() const { return m_selection; }
  QPointF get_insert_position() const;
  QPointF get_insert_position(const QPoint& pos) const;
  QRectF node_geometry(const Node& node) const;
  void populate_context_menu(QMenu& menu) const;
  void pan_to_center();
  bool accepts_paste(const QMimeData& mime_data) const;

public Q_SLOTS:
  void copy_to_clipboard();
  void paste_from_clipboard();

protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent*) override;
  bool event(QEvent* event) override;

private:
  NodeModel* m_model = nullptr;
  PanZoomController m_pzc;
  void draw_node(QPainter& painter, const Node& node) const;
  void draw_connection(QPainter& painter, const InputPort& input_port) const;
  void draw_port(QPainter& painter, const AbstractPort& port, bool text) const;
  void draw_connection(QPainter& painter, const QPointF& in, const QPointF& out, bool is_floating) const;
  QPointF port_pos(const AbstractPort& port) const;
  AbstractPort* port_at(std::set<AbstractPort*> candidates, const QPointF& pos) const;
  Node* node_at(std::set<Node*> candidates, const QPointF& pos) const;
  std::pair<Node*, AbstractPort*> select_port_or_node(const QPointF& pos) const;
  std::set<Node*> nodes(const QRectF& rect) const;
  void update_scene_selection();
  bool can_drop(const QDropEvent& event) const;
  constexpr static auto m_droppable_kinds = Kind::Item;

  class CachedNodeWidthGetter : public ArgsCachedGetter<double, NodeView, const Node*>
  {
  public:
    explicit CachedNodeWidthGetter(NodeView& node_view);
  protected:
    double compute(const Node* node) const override;
  private:
    const QFontMetricsF m_font_metrics;
  } node_width_cache;

  AbstractPort* m_tmp_connection_origin = nullptr;
  AbstractPort* m_tmp_connection_target = nullptr;
  AbstractPort* m_former_connection_target = nullptr;
  InputPort* m_about_to_disconnect = nullptr;
  bool m_aborted = false;
  std::set<Node*> m_selection;
  std::set<Node*> m_nodes_in_rubberband;
  std::map<const Node*, std::map<Property*, std::unique_ptr<QWidget>>> m_property_widgets;
  void update_widgets();

private Q_SLOTS:
  void invalidate_caches();
};

}  // namespace omm
