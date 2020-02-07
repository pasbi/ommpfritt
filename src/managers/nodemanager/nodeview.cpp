#include "managers/nodemanager/nodeview.h"
#include "propertywidgets/propertywidget.h"
#include "commands/nodecommand.h"
#include "managers/nodemanager/nodemimedata.h"
#include <QClipboard>
#include <QApplication>
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

namespace
{

QPoint find_leaf_widget(QWidget* &parent, const QPoint& widget_local_pos)
{
  QPoint pos = widget_local_pos;
  while (true) {
    QWidget* const child = parent->childAt(widget_local_pos);
    if (child == nullptr) {
      break;
    } else {
      pos = child->mapFromParent(pos);
      parent = child;
    }
  }
  return pos - widget_local_pos;
}

omm::AbstractPort* find_port(const omm::Node& node, const omm::Property& property)
{
  if (omm::InputPort* ip = node.find_port<omm::InputPort>(property); ip != nullptr) {
    return ip;
  } else if (omm::OutputPort* op = node.find_port<omm::OutputPort>(property); op != nullptr) {
    return op;
  } else {
    return nullptr;
  }
}

}  // namespace

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
  m_property_widgets.clear();
}

void NodeView::set_model(NodeModel* model)
{
  if (m_model != model) {
    if (m_model != nullptr) {
      disconnect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
      disconnect(m_model, SIGNAL(node_shape_changed()), this, SLOT(invalidate_caches()));
    }
    m_model = model;
    m_property_widgets.clear();
    if (m_model != nullptr) {
      connect(m_model, SIGNAL(appearance_changed()), this, SLOT(update()));
      connect(m_model, SIGNAL(node_shape_changed()), this, SLOT(invalidate_caches()));
    }
  }
  pan_to_center();
}

void NodeView::paintEvent(QPaintEvent*)
{
  update_widgets();
  if (m_model != nullptr) {
    QPainter painter(this);

    {
      const QColor background = m_model->status() == NodeModel::Status::Fail
                                ? ui_color(*this, "NodeView", "canvas-bg-invalid")
                                : ui_color(*this, "NodeView", "canvas-bg-valid");
      painter.fillRect(rect(), background);
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2.0, height()/2.0);
    painter.setTransform(m_pzc.transform(), true);

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
        draw_connection(painter, origin, target, true);
      } else {
        draw_connection(painter, target, origin, true);
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

bool NodeView::accepts_paste(const QMimeData &mime_data) const
{
  if (NodeModel* model = this->model(); model != nullptr) {
    return mime_data.hasFormat(NodeMimeData::MIME_TYPES.at(model->language()));
  } else {
    return false;
  }
}

void NodeView::copy_to_clipboard()
{
  if (NodeModel* model = this->model(); model != nullptr) {
    auto mime_data = std::make_unique<NodeMimeData>(model->language(), selected_nodes());
    QApplication::clipboard()->setMimeData(mime_data.release());
  }
}

void NodeView::paste_from_clipboard()
{
  const QMimeData& mime_data = *QApplication::clipboard()->mimeData();
  if (accepts_paste(mime_data)) {
    NodeModel& model = *this->model();
    const auto nodes = ::transform<Node*, std::vector>(static_cast<const NodeMimeData&>(mime_data).nodes());

    std::map<const Node*, Node*> copy_map;
    auto copies = ::transform<std::unique_ptr<Node>, std::vector>(nodes,
                                                                  [&model, &copy_map](Node* node)
    {
      auto clone = node->clone(model);
      copy_map[node] = clone.get();
      return clone;
    });

    { // set position
      const auto old_center = std::accumulate(nodes.begin(), nodes.end(), QPointF(),
                                          [](const QPointF& p, const Node* node)
      {
        return node->pos() + p;
      }) / nodes.size();
      const auto diff = get_insert_position(mapFromGlobal(QCursor::pos())) - old_center;
      for (auto& node : copies) {
        node->set_pos(node->pos() + diff);
      }
    }

    if (!copies.empty()) {
      Scene& scene = model.scene();

      auto macro = scene.history().start_macro(tr("Copy Nodes"));

      { // restore connections
        for (auto&& [o_target, c_target] : copy_map) {
          for (const InputPort* o_input : o_target->ports<InputPort>()) {
            if (const OutputPort* o_output = o_input->connected_output(); o_output != nullptr) {
              const Node& o_source = o_output->node;
              if (::contains(copy_map, &o_source)) {
                const Node& c_source = *copy_map.at(&o_source);
                OutputPort& c_output = *c_source.find_port<OutputPort>(o_output->index);
                InputPort& c_input = *c_target->find_port<InputPort>(o_input->index);
                scene.submit<ConnectPortsCommand>(c_output, c_input);
              }
            }
          }
        }
      }

      // insert nodes
      scene.submit<AddNodesCommand>(model, std::move(copies));
    }

    m_selection = ::transform<Node*, std::set>(copy_map, [](auto&& pair) { return pair.second; });
  }
}

bool NodeView::can_drop(const QDropEvent& event) const
{
  const QMimeData& mime_data = *event.mimeData();
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

void NodeView::update_widgets()
{
  NodeModel* model = this->model();
  if (model == nullptr) {
    m_property_widgets.clear();
  } else {
    for (auto&& [node, map] : m_property_widgets) {
      if (!::contains(model->nodes(), node)) {
        m_property_widgets.erase(node);
      }
    }
    for (Node* node : model->nodes()) {
      if (!::contains(m_property_widgets, node)) {
        for (Property* property : node->properties().values()) {
          auto widget = AbstractPropertyWidget::make(property->widget_type(),
                                                     model->scene(),
                                                     std::set { property });
          m_property_widgets[node][property] = std::move(widget);
        }
      }
    }
  }
}

QRectF NodeView::widget_geometry(const AbstractPort& p) const
{
  static constexpr QPointF margin(port_height/2.0, 0.5);
  const Node& node = p.node;
  const double width = node_width_cache(&node) - 2*margin.x();
  const double height = port_height - 2*margin.y();
  return QRectF(node.pos().x() - width/2.0,
                port_pos(p).y() - height/2.0,
                width,
                height);
}

NodeView::WidgetInfo NodeView::widget_at(const QPointF& pos) const
{
  for (auto&& [node, map] : m_property_widgets) {
    for (auto&& [property, widget] : map) {
      if (AbstractPort* p = find_port(*node, *property); p != nullptr) {
        if (const QRectF geom = widget_geometry(*p); geom.contains(pos)) {
          return WidgetInfo(pos.toPoint(), *p, geom, widget.get());
        }
      }
    }
  }
  return NodeView::WidgetInfo();
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
      auto node = Node::make(ReferenceNode::TYPE, *model());
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

bool NodeView::forward_mouse_event(const QMouseEvent& event) const
{
  static WidgetInfo wi;
  const QPointF fpos = m_pzc.transform().inverted().map(event.pos() - m_pzc.offset());
  if (event.type() == QEvent::MouseButtonPress || event.type() == QEvent::MouseButtonDblClick) {
    wi = widget_at(fpos);
  }
  if (wi.valid()) {
    wi.update_pos(fpos.toPoint());
    auto new_event = std::make_unique<QMouseEvent>(event.type(),
                                                   wi.leaf_widget_pos,
                                                   event.button(),
                                                   event.buttons(),
                                                   event.modifiers());
    QApplication::sendEvent(wi.leaf_widget, &*new_event);
    if (event.type() == QEvent::MouseButtonRelease) {
      wi = WidgetInfo();
    }
    return true;
  } else {
    return false;
  }
}

bool NodeView::forward_dnd_event(const QDropEvent& event) const
{
  static QWidget* last_widget = nullptr;
  const auto make_event = [](QEvent::Type type, auto&&... args) -> std::unique_ptr<QEvent> {
    switch (type) {
    case QEvent::Drop:
      return std::make_unique<QDropEvent>(std::forward<decltype(args)>(args)...);
    case QEvent::DragMove:
      return std::make_unique<QDragMoveEvent>(std::forward<decltype(args)>(args)...);
    case QEvent::DragEnter:
      return std::make_unique<QDragEnterEvent>(std::forward<decltype(args)>(args)...);
    default:
      Q_UNREACHABLE();
      return nullptr;
    }
  };
  const QPointF fpos = m_pzc.transform().inverted().map(event.pos() - m_pzc.offset());
  const WidgetInfo wi = widget_at(fpos);
  QEvent::Type new_event_type = event.type();
  if (last_widget != wi.leaf_widget) {
    if (last_widget != nullptr) {
      QDragLeaveEvent leave_event;
      QApplication::sendEvent(wi.leaf_widget, &leave_event);
    }
    if (wi.leaf_widget != nullptr) {
      new_event_type = QEvent::DragEnter;
    }
  }

  if (wi.valid()) {
    auto new_event = make_event(new_event_type, wi.leaf_widget_pos, event.possibleActions(),
                                event.mimeData(), event.mouseButtons(), event.keyboardModifiers());
    QApplication::sendEvent(wi.leaf_widget, &*new_event);
    return true;
  } else {
    return false;
  }
}

void NodeView::mouseDoubleClickEvent(QMouseEvent*)
{
  pan_to_center();
}

bool NodeView::event(QEvent* event)
{
  static const std::set mouse_event_types { QEvent::MouseButtonPress, QEvent::MouseButtonRelease,
                                            QEvent::MouseButtonDblClick, QEvent::MouseMove        };

  static const std::set dnd_even_types { QEvent::Drop, QEvent::DragMove };
  if (event->type() == QEvent::ToolTip) {
    if (m_model != nullptr) {
      const QHelpEvent* help_event = static_cast<const QHelpEvent*>(event);
      auto [ node, port ] = select_port_or_node(help_event->pos() - m_pzc.offset());
      if (node != nullptr) {
        QToolTip::showText(help_event->globalPos(), "node");
      } else if (port != nullptr) {
        const QString tooltip = QString("port\n%1\n%2")
            .arg(port->data_type()).arg(port->uuid());
        QToolTip::showText(help_event->globalPos(), tooltip);
      } else {
        QToolTip::hideText();
      }
    }
  } else if (::contains(mouse_event_types, event->type())) {
    if (forward_mouse_event(static_cast<const QMouseEvent&>(*event))) {
      update();
      return true;
    }
  } else if (::contains(dnd_even_types, event->type())) {
    if (forward_dnd_event(static_cast<QDropEvent&>(*event))) {
      update();
      return true;
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
  const QRectF node_geometry = this->node_geometry(node);
  painter.save();
  QPen pen;
  pen.setWidthF(2.2);
  if (node.is_valid()) {
    pen.setColor(ui_color(*this, "NodeView", "node-outline-valid"));
  } else {
    pen.setColor(ui_color(*this, "NodeView", "node-outline-invalid"));
  }
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
    draw_port(painter, *port, port->flavor != PortFlavor::Property);
  }

  const auto pw_it = m_property_widgets.find(&node);
  if (pw_it != m_property_widgets.end()) {
    for (auto&& [property, widget] : pw_it->second) {
      if (AbstractPort* p = find_port(node, *property); p != nullptr) {
        painter.save();
        const QRectF geometry = widget_geometry(*p);
        painter.translate(geometry.topLeft());
        widget->move(mapToGlobal((m_pzc.transform().map(geometry.topLeft()) + m_pzc.offset()).toPoint()));
        widget->resize(geometry.size().toSize());
        widget->render(&painter);
        painter.restore();
      }
    }
  }
}

void NodeView::draw_connection(QPainter& painter, const InputPort& input_port) const
{
  if (&input_port != m_about_to_disconnect) {
    const OutputPort* op = input_port.connected_output();
    if (op != nullptr) {
      draw_connection(painter, port_pos(input_port), port_pos(*op), false);
    }
  }
}

void NodeView::draw_port(QPainter& painter, const AbstractPort& port, bool text) const
{
  painter.save();
  const double r = 0.8 * (port_height / 2.0);

  QPen pen;
  pen.setWidthF(2.2);
  pen.setColor(Qt::black);
  painter.setPen(pen);

  QBrush brush(ui_color(*this, "NodeView", QString("port-%1").arg(port.data_type())));
  painter.setBrush(brush);

  const QPointF port_pos = this->port_pos(port);
  const QPointF node_pos = port.node.pos();
  painter.drawEllipse(port_pos, r, r);

  if (text) {
    const double ph = port_height;
    const double width = node_width_cache(&port.node);
    const QRectF text_rect(node_pos.x() - width/2.0 + ph/2.0 + margin,
                           port_pos.y() - ph/2.0, width - ph - margin*2.0, ph);
    const auto halign = (port.port_type == PortType::Input ? Qt::AlignLeft : Qt::AlignRight);
    painter.drawText(text_rect, halign | Qt::AlignVCenter, port.label());
  }
  painter.restore();
}

void NodeView::draw_connection(QPainter& painter, const QPointF& in, const QPointF& out,
                               bool is_floating) const
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
  if (is_floating) {
    // floating
    QPen pen;
    pen.setWidth(2.0);
    pen.setColor(ui_color(*this, "NodeView", "floating-connection"));
    painter.setPen(pen);
    painter.drawPath(path);
  } else {
    // fixed
    QPen pen;
    pen.setWidth(2.0);
    pen.setColor(ui_color(*this, "NodeView", "fixed-connection"));
    painter.setPen(pen);
    painter.drawPath(path);
  }
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

NodeView::WidgetInfo::WidgetInfo(const QPoint& pos, const AbstractPort& port, const QRectF& geom, QWidget* widget)
  : widget(widget)
  , widget_pos(pos)
  , port(&port)
{
  leaf_widget_pos = (pos - geom.topLeft()).toPoint();
  leaf_widget = widget;
  find_leaf_widget(leaf_widget, leaf_widget_pos);
}

void NodeView::WidgetInfo::update_pos(const QPoint& pos)
{
  const QPoint diff = pos - widget_pos;
  widget_pos = pos;
  leaf_widget_pos += diff;
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
