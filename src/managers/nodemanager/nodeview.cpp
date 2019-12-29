#include "managers/nodemanager/nodeview.h"
#include "properties/referenceproperty.h"
#include <QMimeData>
#include "scene/propertyownermimedata.h"
#include "managers/nodemanager/nodes/referencenode.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "commands/nodecommand.h"
#include "managers/nodemanager/node.h"
#include "mainwindow/application.h"
#include "preferences/preferences.h"
#include "managers/nodemanager/nodemodel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace omm
{

static constexpr double port_height = 20;
static constexpr double node_header_height = 20;
static constexpr double node_footer_height = 10;
static constexpr double margin = 5.0;

NodeView::NodeView(QWidget* parent)
  : QWidget(parent)
  , m_pzc(*this)
  , node_width_cache(*this)
{
  setAcceptDrops(true);
  setMouseTracking(true);
}

NodeView::~NodeView()
{
}

void NodeView::set_model(NodeModel* model)
{
  if (m_model != model) {
    if (m_model != nullptr) {
      disconnect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
      disconnect(m_model, SIGNAL(node_shape_changed()), this, SLOT(invalidate_caches()));
    }
    m_model = model;
    if (m_model != nullptr) {
      connect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
      connect(m_model, SIGNAL(node_shape_changed()), this, SLOT(invalidate_caches()));
    }
  }
  pan_to_center();
}

void NodeView::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.translate(width()/2.0, height()/2.0);
  painter.setTransform(m_pzc.transform(), true);

  if (m_model != nullptr) {
    const auto nodes = m_model->nodes();
    for (const auto& node : nodes) {
      for (AbstractPort* port : node->ports()) {
        if (port->port_type == PortType::Input) {
          draw_connection(painter, *static_cast<InputPort*>(port));
        }
      }
    }
    for (auto nit = nodes.rbegin(); nit != nodes.rend(); ++nit) {
      const Node& node = **nit;
      draw_node(painter, node);
    }
    if (!m_aborted && m_tmp_connection_origin != nullptr) {
      const QPointF origin = port_pos(*m_tmp_connection_origin);
      const QPointF target = m_tmp_connection_target == nullptr
          ? m_pzc.transform().inverted().map(QPointF(m_pzc.last_mouse_pos()))
          : port_pos(*m_tmp_connection_target);
      if (m_tmp_connection_origin->port_type == PortType::Input) {
        draw_connection(painter, origin, target);
      } else {
        draw_connection(painter, target, origin);
      }
    }
    m_pzc.draw_rubberband(painter);
  }
}

std::pair<Node*, AbstractPort*> NodeView::select_port_or_node(const QPointF& pos) const
{
  for (Node* node : m_model->nodes()) {
    AbstractPort* port = port_at(node->ports(), pos);
    if (port != nullptr) {
      return { nullptr, port };
    } else if (node_at({ node }, pos) != nullptr) {
      return { node, nullptr };
    }
  }
  return { nullptr, nullptr };
}

std::set<Node*> NodeView::nodes(const QRectF& rect) const
{
  if (m_model == nullptr) {
    return {};
  } else {
    std::set<Node*> nodes;
    for (Node* node : m_model->nodes()) {
      if (node_geometry(*node).intersects(rect)) {
        nodes.insert(node);
      }
    }
    return nodes;
  }
}

void NodeView::update_scene_selection()
{
  m_model->scene().set_selection(::transform<AbstractPropertyOwner*>(m_selection, ::identity));
}

bool NodeView::can_drop(const QDropEvent& event) const
{
  const auto& mime_data = *event.mimeData();
  if (event.dropAction() != Qt::LinkAction) {
    return false;
  } else if (mime_data.hasFormat(PropertyOwnerMimeData::MIME_TYPE)) {
    const auto property_owner_mime_data = qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    if (property_owner_mime_data != nullptr) {
      if (property_owner_mime_data->items(m_droppable_kinds).size() >= 1) {
        return true;
      }
    }
  }
  return false;
}

void NodeView::invalidate_caches()
{
  node_width_cache.invalidate();
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
  m_aborted = false;
  m_pzc.move(event->pos());
  if (m_model != nullptr) {
    if (preferences().match("shift viewport", *event, true)) {
      m_pzc.start(PanZoomController::Action::Pan);
    } else if (preferences().match("zoom viewport", *event, true)) {
      m_pzc.start(PanZoomController::Action::Zoom);
    } else {
      m_pzc.start(PanZoomController::Action::None);
      const bool extend_selection = event->modifiers() & Qt::ShiftModifier;
      const bool toggle_selection = event->modifiers() & Qt::ControlModifier;
      const auto [ node, port ] = select_port_or_node(event->pos() - m_pzc.offset());

      const auto press_on_port = [this](AbstractPort* port) {
        if (port->port_type == PortType::Input) {
          InputPort& ip = static_cast<InputPort&>(*port);
          if (OutputPort* op = ip.connected_output(); op == nullptr) {
            m_tmp_connection_origin = &ip;
          } else {
            m_former_connection_target = &ip;
            m_about_to_disconnect = &ip;
            m_tmp_connection_origin = op;
          }
        } else {
          m_tmp_connection_origin = port;
        }
      };

      const auto press_on_node = [toggle_selection, extend_selection, this](Node* node) {
        const bool is_selected = ::contains(m_selection, node);
        if (toggle_selection) {
          if (is_selected) {
            m_selection.erase(node);
          } else {
            m_selection.insert(node);
          }
        } else {
          if (!extend_selection && !is_selected) {
            m_selection.clear();
          }
          m_selection.insert(node);
        }
        update_scene_selection();
      };

      if (node != nullptr || port != nullptr) {
        if (node != nullptr) {
          press_on_node(node);
        } else if (port != nullptr) {
          press_on_port(port);
        }
        if (event->button() == Qt::RightButton) {
          m_aborted = true;
          QWidget::mousePressEvent(event);
        } else {
          event->accept();
        }
      } else if (event->button() == Qt::LeftButton) {
        m_pzc.rubber_band_visible = true;
        if (!extend_selection) {
          m_selection.clear();
          update_scene_selection();
        }
      } else {
        QWidget::mousePressEvent(event);
      }
    }
    update();
  } else {
    QWidget::mousePressEvent(event);
  }
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  QToolTip::hideText();
  if (!m_aborted && event->buttons() != 0) {
    if (m_pzc.move(event->pos())) {
    } else if (m_pzc.rubber_band_visible) {
      m_nodes_in_rubberband = this->nodes(m_pzc.unit_rubber_band());
    } else if (m_tmp_connection_origin == nullptr && m_model != nullptr) {
      if (event->buttons() & Qt::LeftButton) {
        if (const QPointF e = m_pzc.unit_d(); e.manhattanLength() > 0 && !m_selection.empty()) {
          m_model->scene().submit<MoveNodesCommand>(m_selection, e);
        }
      }
    } else if (m_tmp_connection_origin != nullptr && m_model != nullptr) {
      AbstractPort* port = this->port_at(m_model->ports(), event->pos() - m_pzc.offset());
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

  if (m_pzc.rubber_band_visible) {
    m_pzc.rubber_band_visible = false;
    m_selection.insert(m_nodes_in_rubberband.begin(), m_nodes_in_rubberband.end());
    update_scene_selection();
    m_nodes_in_rubberband.clear();
  }

  {
    std::unique_ptr<Macro> macro;
    if (commands.size() > 1) {
      macro = m_model->scene().history().start_macro(tr("Modify Connections"));
    }
    for (auto&& command : commands) {
      m_model->scene().submit(std::move(command));
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

void NodeView::dragEnterEvent(QDragEnterEvent* event)
{
  event->setDropAction(Qt::LinkAction);
  if (can_drop(*event)) {
    event->accept();
  } else {
    event->ignore();
  }
}

void NodeView::dropEvent(QDropEvent* event)
{
  event->setDropAction(Qt::LinkAction);
  if (can_drop(*event)) {
    const auto& mime_data = *event->mimeData();
    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    const auto items = property_owner_mime_data.items(m_droppable_kinds);

    std::vector<std::unique_ptr<Node>> nodes;
    nodes.reserve(items.size());
    Scene& scene = model()->scene();
    QPointF insert_pos = get_insert_position(event->pos());
    for (AbstractPropertyOwner* item : items) {
      auto node = Node::make(ReferenceNode::TYPE, &scene);
      auto* const property = node->property(ReferenceNode::REFERENCE_PROPERTY_KEY);
      property->set(item);
      const QSizeF size = node_geometry(*node).size();
      insert_pos -= QPointF(size.width(), size.height()) / 2.0;
      node->set_pos(insert_pos);
      nodes.push_back(std::move(node));
    }
    scene.submit<AddNodesCommand>(*model(), std::move(nodes));
  } else {
    event->ignore();
  }
}

void NodeView::mouseDoubleClickEvent(QMouseEvent*)
{
  pan_to_center();
}

bool NodeView::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip) {
    const QHelpEvent* help_event = static_cast<const QHelpEvent*>(event);
    auto [ node, port ] = select_port_or_node(help_event->pos() - m_pzc.offset());
    if (node != nullptr) {
      QToolTip::showText(help_event->globalPos(), "node");
    } else if (port != nullptr) {
      const QString tooltip = QString("port\n%1")
          .arg(port->data_type());
      QToolTip::showText(help_event->globalPos(), tooltip);
    } else {
      QToolTip::hideText();
    }
  }

  return QWidget::event(event);
}

void NodeView::abort()
{
  m_aborted = true;
  m_pzc.rubber_band_visible = false;
  m_nodes_in_rubberband.clear();
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
    m_model->scene().submit<RemoveNodesCommand>(*m_model, selection);
  }
}

QPointF NodeView::get_insert_position(const QPoint& pos) const
{
  return m_pzc.transform().inverted().map(pos - m_pzc.offset());
}

QPointF NodeView::get_insert_position() const
{
  return m_pzc.transform().inverted().map(m_pzc.last_mouse_pos());
}

void NodeView::draw_node(QPainter& painter, const Node& node) const
{
  painter.save();

  const QRectF node_geometry = this->node_geometry(node);
  painter.save();
  QPen pen;
  pen.setWidthF(2.2);
  pen.setColor(Qt::black);
  painter.setPen(pen);
  if (::contains(m_selection, &node) || ::contains(m_nodes_in_rubberband, &node)) {
    painter.setBrush(QBrush(QColor(210, 210, 50)));
  } else {
    painter.setBrush(QBrush(QColor(180, 180, 180)));
  }
  painter.drawRoundedRect(node_geometry, 5, 5, Qt::AbsoluteSize);

  const QRectF header_rect(node_geometry.topLeft(),
                           QSizeF(node_geometry.width(), node_header_height));
  painter.drawText(header_rect, Qt::AlignVCenter | Qt::AlignHCenter, node.title());
  painter.restore();


  for (AbstractPort* port : node.ports()) {
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

void NodeView::draw_port(QPainter& painter, const AbstractPort& port) const
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
  const QRectF text_rect(node_pos.x() - width/2.0 + ph/2.0 + margin,
                         port_pos.y() - ph/2.0, width - ph - margin*2.0, ph);
  const auto halign = (port.port_type == PortType::Input ? Qt::AlignLeft : Qt::AlignRight);
  painter.drawText(text_rect, halign | Qt::AlignVCenter, port.label());
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

AbstractPort* NodeView::port_at(std::set<AbstractPort*> candidates, const QPointF& pos) const
{
  static constexpr bool NODE_SELECTION_IN_PIXEL_SPACE = false;
  if constexpr (NODE_SELECTION_IN_PIXEL_SPACE) {
    const auto dist = [pos, this](const QPointF& candidate) -> double {
      return (pos - m_pzc.transform().map(candidate)).manhattanLength();
    };
    for (AbstractPort* port : candidates) {
      if (dist(port_pos(*port)) < 5) {
        return port;
      }
    }
  } else {
    const QPointF mpos = m_pzc.transform().inverted().map(pos);
    for (AbstractPort* port : candidates) {
      if ((mpos - port_pos(*port)).manhattanLength() < port_height / 2.0) {
        return port;
      }
    }
  }
  return nullptr;
}

Node* NodeView::node_at(std::set<Node*> candidates, const QPointF& pos) const
{
  for (Node* node : candidates) {
    if (m_pzc.transform().map(node_geometry(*node)).containsPoint(pos, Qt::OddEvenFill)) {
      return node;
    }
  }
  return nullptr;
}

QRectF NodeView::node_geometry(const Node& node) const
{
  const std::set<AbstractPort*> ports = node.ports();
  std::size_t n = 0;
  for (const AbstractPort* port : ports) {
    n = std::max(port->index + 1, n);
  }
  const double height = node_header_height + node_footer_height + n * port_height;
  const QPointF offset(node_width_cache(&node), height);
  return QRectF(node.pos() - offset/2.0, node.pos() + offset/2.0);
}

void NodeView::populate_context_menu(QMenu& menu) const
{
  if (m_selection.size() == 1) {
    (**m_selection.begin()).populate_menu(menu);
  }
}

void NodeView::pan_to_center()
{
  if (m_model != nullptr) {
    const auto nodes = m_model->nodes();
    QPointF mean(0.0, 0.0);
    for (Node* node : nodes) {
      mean += node->pos();
    }
    mean /= std::max(std::size_t(1), nodes.size());

    m_pzc.translate(-m_pzc.transform().map(mean));
    update();
  }
}

QPointF NodeView::port_pos(const AbstractPort& port) const
{
  const QRectF node_geometry = this->node_geometry(port.node);
  const double y = (port.index+0.5) * port_height + node_header_height - node_geometry.height()/2.0;
  const double x = (port.port_type == PortType::Input ? -0.5 : 0.5) * node_geometry.width();
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
  for (const AbstractPort* port : node->ports()) {
    auto it = widths.find(port->index);
    if (it == widths.end()) {
      it = widths.insert({port->index, 0.0}).first;
    }
    it->second += m_font_metrics.horizontalAdvance(port->label());
  }
  double max_width = 0.0;
  for (auto&& [index, width] : widths) {
    max_width = std::max(max_width, width);
  }

  const double header_width = m_font_metrics.horizontalAdvance(node->title());

  return std::max(max_width + port_height + margin, header_width) + margin * 2.0;
}

}  // namespace omm
