#include "managers/nodemanager/nodeview.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "commands/nodecommand.h"
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
      const QPointF origin = port_pos(*m_tmp_connection_origin);
      const QPointF target = m_tmp_connection_target == nullptr
          ? m_pzc.transform().inverted().map(QPointF(m_pzc.last_mouse_pos()))
          : port_pos(*m_tmp_connection_target);
      if (m_tmp_connection_origin->is_input) {
        draw_connection(painter, origin, target);
      } else {
        draw_connection(painter, target, origin);
      }
    }
  }
}

bool NodeView::select_port_or_node(const QPoint pos, bool extend_selection)
{
  const auto contains = [pos, this](const QRectF& rect) {
    return m_pzc.transform().map(rect).containsPoint(pos, Qt::OddEvenFill);
  };
  for (Node* node : m_model->nodes()) {
    if (Port* grabbed_port = port(node->ports(), pos); grabbed_port != nullptr) {
      if (grabbed_port->is_input) {
        InputPort& ip = static_cast<InputPort&>(*grabbed_port);
        if (OutputPort* op = ip.connected_output(); op == nullptr) {
          m_tmp_connection_origin = &ip;
        } else {
          m_former_connection_target = &ip;
          m_about_to_disconnect = &ip;
          m_tmp_connection_origin = op;
        }
      } else {
        m_tmp_connection_origin = grabbed_port;
      }
      return true;
    } else {
      if (contains(node_geometry(*node))) {
        if (!extend_selection && !::contains(m_selection, node)) {
          m_selection.clear();
        }
        m_selection.insert(node);
        return true;
      }
    }
  }
  m_selection.clear();
  return false;
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
  m_aborted = false;
  m_pzc.move(event->pos() - offset());
  if (preferences().match("shift viewport", *event, true)) {
    m_pzc.start(PanZoomController::Action::Pan);
  } else if (preferences().match("zoom viewport", *event, true)) {
    m_pzc.start(PanZoomController::Action::Zoom);
  } else if (event->button() == Qt::LeftButton && m_model != nullptr) {
    select_port_or_node(event->pos() - offset(), event->modifiers() & Qt::ShiftModifier);
  }
  update();
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_aborted) {
    if (m_pzc.move(event->pos() - offset())) {
    } else if (m_tmp_connection_origin == nullptr && m_model != nullptr) {
      if (event->buttons() & Qt::LeftButton) {
        if (const QPointF e = m_pzc.e(); e.manhattanLength() > 0 && !m_selection.empty()) {
          m_model->scene()->submit<MoveNodesCommand>(m_selection, m_pzc.e());
        }
      }
    } else if (m_tmp_connection_origin != nullptr && m_model != nullptr) {
      Port* port = this->port(m_model->ports(), event->pos() - offset());
      if (port != nullptr && m_model->can_connect(*port, *m_tmp_connection_origin)) {
        m_tmp_connection_target = port;
      } else {
        m_tmp_connection_target = nullptr;
      }
    }
    update();
  }
}

void NodeView::mouseReleaseEvent(QMouseEvent*)
{
  std::list<std::unique_ptr<Command>> commands;
  if (m_tmp_connection_origin != nullptr && m_tmp_connection_target != nullptr) {
    if (m_model != nullptr) {
      commands.push_back(std::make_unique<ConnectPortsCommand>(*m_tmp_connection_origin,
                                                               *m_tmp_connection_target));
      if (m_about_to_disconnect == m_tmp_connection_origin
          || m_about_to_disconnect == m_tmp_connection_target)
      {
        m_about_to_disconnect = nullptr;
      }
    }
  }
  if (m_about_to_disconnect != nullptr) {
    commands.push_back(std::make_unique<DisconnectPortsCommand>(*m_about_to_disconnect));
  }

  {
    std::unique_ptr<Macro> macro;
    if (commands.size() > 1) {
      macro = m_model->scene()->history().start_macro(tr("Modify Connections"));
    }
    for (auto&& command : commands) {
      m_model->scene()->submit(std::move(command));
    }
  }

  m_former_connection_target = nullptr;
  m_tmp_connection_origin = nullptr;
  m_tmp_connection_target = nullptr;
  m_about_to_disconnect = nullptr;
  m_aborted = false;
  m_pzc.end();
  update();
}

void NodeView::abort()
{
  m_aborted = true;
  m_about_to_disconnect = nullptr;
  m_tmp_connection_origin = nullptr;
  m_tmp_connection_target = nullptr;
  m_former_connection_target = nullptr;
  update();
}

void NodeView::remove_selection()
{
  if (m_model != nullptr) {
    const auto selection = ::transform<Node*, std::vector>(m_selection);
    m_model->scene()->submit<RemoveNodesCommand>(*m_model, selection);
  }
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
  if (&input_port != m_about_to_disconnect) {
    const OutputPort* op = input_port.connected_output();
    if (op != nullptr) {
      draw_connection(painter, port_pos(input_port), port_pos(*op));
    }
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

void NodeView::draw_connection(QPainter& painter, const QPointF& in, const QPointF& out) const
{
#define INTERPOLATE_CONNECTION_AWAY_FROM_NODE
#if defined(INTERPOLATE_CONNECTION_CORRECT_DIRECTION)
  const QPointF c1((2.0 * in.x() + 1.0 * out.x())/3.0, in.y());
  const QPointF c2((1.0 * in.x() + 2.0 * out.x())/3.0, out.y());
#elif defined(INTERPOLATE_CONNECTION_AWAY_FROM_NODE)
  const double d = 2.0 / 3.0 * abs(in.x() - out.x());
  const QPointF c1(in.x() - d, in.y());
  const QPointF c2(out.x() + d, out.y());
#endif
  QPainterPath path;
  path.moveTo(in);
  path.cubicTo(c1, c2, out);

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

Port* NodeView::port(std::set<Port*> candidates, const QPointF& pos) const
{
  const auto d = [pos, this](const QPointF& candidate) {
    return (pos - m_pzc.transform().map(candidate)).manhattanLength();
  };
  for (Port* port : candidates) {
    if (d(port_pos(*port)) < port_height / 2.0) {
      return port;
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
