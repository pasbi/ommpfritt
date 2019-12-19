#pragma once

#include "managers/panzoomcontroller.h"
#include "cachedgetter.h"
#include <QWidget>
#include "managers/range.h"

class QPainter;
class QMenu;

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
  QRectF node_geometry(const Node& node) const;
  void populate_context_menu(QMenu& menu) const;

protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent*) override;

private:
  NodeModel* m_model = nullptr;
  PanZoomController m_pzc;
  void draw_node(QPainter& painter, const Node& node) const;
  void draw_connection(QPainter& painter, const InputPort& input_port) const;
  void draw_port(QPainter& painter, const AbstractPort& port) const;
  void draw_connection(QPainter& painter, const QPointF& in, const QPointF& out) const;
  QPointF port_pos(const AbstractPort& port) const;
  AbstractPort* port(std::set<AbstractPort*> candidates, const QPointF& pos) const;
  bool select_port_or_node(const QPointF& pos, bool extend_selection, bool toggle_selection);
  std::set<Node*> nodes(const QRectF& rect) const;
  QString header_text(const Node& node) const;
  void update_scene_selection();

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
};

}  // namespace omm
