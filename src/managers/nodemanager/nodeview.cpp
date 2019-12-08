#include "managers/nodemanager/nodeview.h"
#include "managers/nodemanager/node.h"
#include "mainwindow/application.h"
#include "preferences/preferences.h"
#include "managers/nodemanager/nodemodel.h"
#include <QMouseEvent>
#include <QPainter>

namespace omm
{

static constexpr double port_height = 40;
static constexpr double node_header_height = 20;
static constexpr double node_footer_height = 10;
static constexpr double margin = 5.0;

NodeView::NodeView(QWidget* parent)
  : QWidget(parent)
  , node_width_cache(*this)
{
}

NodeView::~NodeView()
{
}

void NodeView::set_model(NodeModel* model)
{
  if (m_model != model) {
    if (m_model != nullptr) {
      disconnect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
    }
    m_model = model;
    if (m_model != nullptr) {
      connect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
    }
    update();
  }
}

void NodeView::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.translate(width()/2.0, height()/2.0);
  painter.setTransform(m_pzc.transform(), true);

  if (m_model != nullptr) {
    const auto nodes = m_model->nodes();
    for (const auto& node : nodes) {
      for (Port* port : node->ports()) {
        if (port->is_input) {
          draw_connection(painter, *static_cast<InputPort*>(port));
        }
      }
    }
    for (auto nit = nodes.rbegin(); nit != nodes.rend(); ++nit) {
      const Node& node = **nit;
      draw_node(painter, node);
    }
    if (m_tmp_connection_origin != nullptr) {
      if (m_tmp_connection_target == nullptr) {
        draw_connection(painter, port_pos(*m_tmp_connection_origin),
                        m_pzc.transform().inverted().map(QPointF(m_pzc.last_mouse_pos())));
      } else {
        draw_connection(painter, port_pos(*m_tmp_connection_origin),
                                 port_pos(*m_tmp_connection_target));
      }
    }
  }
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
  m_pzc.move(event->pos() - offset());
  if (preferences().match("shift viewport", *event, true)) {
    m_pzc.start(PanZoomController::Action::Pan);
  } else if (preferences().match("zoom viewport", *event, true)) {
    m_pzc.start(PanZoomController::Action::Zoom);
  } else if (event->button() == Qt::LeftButton) {
    if (m_model != nullptr) {
      [this, event]() {
        const auto contains = [event, this](const QRectF& rect) {
          return m_pzc.transform().map(rect).containsPoint(event->pos() - offset(), Qt::OddEvenFill);
        };
        m_tmp_connection_origin = port(event->pos());
        for (Node* node : m_model->nodes()) {
          if (contains(node_geometry(*node))) {
            if (!(event->modifiers() & Qt::ShiftModifier) && !::contains(m_selection, node)) {
              m_selection.clear();
            }
            m_selection.insert(node);
            return;
          }
        }
        m_selection.clear();
      }();
    }
  }
  update();
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  if (m_pzc.move(event->pos() - offset())) {
  } else if (m_tmp_connection_origin == nullptr && m_model != nullptr) {
    if (event->buttons() & Qt::LeftButton) {
      for (Node* node : m_selection) {
        node->set_pos(node->pos() + m_pzc.d());
      }
    }
  } else if (m_tmp_connection_origin != nullptr && m_model != nullptr) {
    Port* port = this->port(event->pos());
    if (port != nullptr && m_model->can_connect(*port, *m_tmp_connection_origin)) {
      m_tmp_connection_target = port;
    } else {
      m_tmp_connection_target = nullptr;
    }
  }
  update();
}

void NodeView::mouseReleaseEvent(QMouseEvent*)
{
  if (m_tmp_connection_origin != nullptr && m_tmp_connection_target != nullptr) {
    if (m_model != nullptr) {
      m_model->connect(*m_tmp_connection_origin, *m_tmp_connection_target);
    }
  }
  m_tmp_connection_origin = nullptr;
  m_tmp_connection_target = nullptr;
  m_pzc.end();
  update();
}

QPoint NodeView::offset() const
{
  return QPoint(width(), height()) / 2;
}

void NodeView::draw_node(QPainter& painter, const Node& node) const
{
  painter.save();

  {
    painter.save();
    QPen pen;
    pen.setWidthF(2.2);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    if (::contains(m_selection, &node)) {
      painter.setBrush(QBrush(QColor(210, 210, 50)));
    } else {
      painter.setBrush(QBrush(QColor(180, 180, 180)));
    }
    painter.drawRoundedRect(node_geometry(node), 5, 5, Qt::AbsoluteSize);
    painter.restore();
  }

  for (Port* port : node.ports()) {
    draw_port(painter, *port);
  }

  painter.restore();
}

void NodeView::draw_connection(QPainter& painter, const InputPort& input_port) const
{
  const OutputPort* op = input_port.connected_output();
  if (op != nullptr) {
    draw_connection(painter, port_pos(input_port), port_pos(*op));
  }
}

void NodeView::draw_port(QPainter& painter, const Port& port) const
{
  painter.save();
  const double r = 0.8 * (port_height / 2.0);

  QPen pen;
  pen.setWidthF(2.2);
  pen.setColor(Qt::black);
  painter.setPen(pen);

  QBrush brush(QColor(255, 40, 80));
  painter.setBrush(brush);

  const QPointF port_pos = this->port_pos(port);
  const QPointF node_pos = port.node.pos();
  painter.drawEllipse(port_pos, r, r);

  const double ph = port_height;
  const double width = node_width_cache(&port.node);
  const QRectF text_rect(node_pos.x() + ph/2.0 + margin,
                         port_pos.y() - ph/2.0, width - ph - margin*2.0, ph);
  const auto flags = (port.is_input ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter;
  painter.drawText(text_rect, flags, port.name);
  painter.restore();
}

void NodeView::draw_connection(QPainter& painter, const QPointF& p1, const QPointF& p2) const
{
  const QPointF c1((2.0 * p1.x() + 1.0 * p2.x())/3.0, p1.y());
  const QPointF c2((1.0 * p1.x() + 2.0 * p2.x())/3.0, p2.y());
  QPainterPath path;
  path.moveTo(p1);
  path.cubicTo(c1, c2, p2);

  painter.save();
  QPen pen;
  pen.setWidth(5.0);
  pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawPath(path);
  pen.setWidth(2.0);
  pen.setColor(Qt::black);
  painter.setPen(pen);
  painter.drawPath(path);
  painter.restore();
}

QRectF NodeView::node_geometry(const Node& node) const
{
  const std::set<Port*> ports = node.ports();
  std::size_t n = 0;
  for (const Port* port : ports) {
    n = std::max(port->index + 1, n);
  }
  const double height = node_header_height + node_footer_height + n * port_height;
  return QRectF(node.pos(), QSizeF(node_width_cache(&node), height));
}

Port* NodeView::port(const QPointF& pos) const
{
  const auto d = [pos, this](const QPointF& candidate) {
    return (pos - offset() - m_pzc.transform().map(candidate)).manhattanLength();
  };
  for (Node* node : m_model->nodes()) {
    for (Port* port : node->ports()) {
      if (d(port_pos(*port)) < port_height / 2.0) {
        return port;
      }
    }
  }
  return nullptr;
}

QPointF NodeView::port_pos(const Port& port) const
{
  const double y = (port.index+0.5) * port_height + node_header_height;
  const double x = port.is_input ? 0.0 : node_width_cache(&port.node);
  return port.node.pos() + QPointF(x, y);
}

NodeView::CachedNodeWidthGetter::CachedNodeWidthGetter(NodeView& node_view)
  : ArgsCachedGetter<double, NodeView, const Node *>(node_view)
  , m_font_metrics(node_view.font, &node_view)
{
}

double NodeView::CachedNodeWidthGetter::compute(const Node* node) const
{
  std::map<std::size_t, double> widths;
  for (const Port* port : node->ports()) {
    auto it = widths.find(port->index);
    if (it == widths.end()) {
      it = widths.insert({port->index, 0.0}).first;
    }
    it->second += m_font_metrics.horizontalAdvance(port->name);
  }
  double max_width = 0.0;
  for (auto&& [index, width] : widths) {
    max_width = std::max(max_width, width);
  }
  return max_width + port_height + margin * 3.0;
}

}  // namespace omm
