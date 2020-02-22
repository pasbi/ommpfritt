#include "managers/nodemanager/nodeview.h"
#include "managers/nodemanager/nodeitem.h"
#include "managers/nodemanager/portitem.h"
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
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include "managers/nodemanager/nodemodel.h"

namespace
{

omm::PortItem* get_port_item(omm::NodeModel& model, omm::AbstractPort& port)
{
  omm::NodeItem& node_item = model.node_item(port.node);
  return node_item.port_item(port);
}

}  // namespace

namespace omm
{


NodeView::NodeView(QWidget* parent)
  : QGraphicsView(parent)
  , m_pan_zoom_controller(*this)
{
  setAcceptDrops(true);
  setMouseTracking(true);
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  setDragMode(QGraphicsView::RubberBandDrag);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTransformationAnchor(QGraphicsView::NoAnchor);
  {
    QTransform t;
    t.translate(viewport()->width() / 2.0, viewport()->height() / 2.0);
    setTransform(t.inverted());
  }
  reset_scene_rect();
  m_viewport_center = mapToScene(viewport()->rect().center());
}

NodeView::~NodeView()
{
}

void NodeView::set_model(NodeModel *model)
{
  setScene(model);
  m_model = model;
  const QRectF scene_rect = viewport()->rect();
  setSceneRect(scene_rect);
  pan_to_center();
}

NodeModel* NodeView::model() const
{
  return m_model;
}

std::set<Node*> NodeView::selected_nodes() const
{
  if (m_model == nullptr) {
    return {};
  } else {
    return ::filter_if(m_model->nodes(), [this](Node* node) {
      return m_model->node_item(*node).isSelected();
    });
  }
}

bool NodeView::accepts_paste(const QMimeData &mime_data) const
{
  if (NodeModel* model = this->model(); model != nullptr) {
    return mime_data.hasFormat(NodeMimeData::MIME_TYPES.at(model->language()));
  } else {
    return false;
  }
}

void NodeView::reset_scene_rect()
{
  const QRectF vr = viewport()->rect();
  const QTransform ti = transform().inverted();
  setSceneRect(QRectF(ti.map(vr.topLeft()), ti.map(vr.bottomRight())));
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
      const auto diff = mapToScene(mapFromGlobal(QCursor::pos())) - old_center;
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
  }
}

void NodeView::drawForeground(QPainter* painter, const QRectF&)
{
  static const auto reverse_connection = [](const PortItem& origin) {
    return origin.port.port_type == PortType::Output;
  };
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setTransform(viewportTransform());
  if (NodeModel* model = this->model(); model != nullptr) {
    for (Node* node : model->nodes()) {
      for (InputPort* ip : node->ports<InputPort>()) {
        if (OutputPort* op = ip->connected_output(); op != nullptr) {
          const QPointF input_port_item_pos = get_port_item(*m_model, *ip)->scenePos();
          const QPointF output_port_item_pos = get_port_item(*m_model, *op)->scenePos();
          draw_connection(*painter, input_port_item_pos, output_port_item_pos, false, false);
        }
      }
    }
  }

  if (m_tmp_connection_origin != nullptr) {
    const QPointF o_pos = m_tmp_connection_origin->scenePos();
    const bool reverse = reverse_connection(*m_tmp_connection_origin);
    if (m_tmp_connection_target != nullptr) {
      draw_connection(*painter, o_pos, m_tmp_connection_target->scenePos(), true, reverse);
    } else {
      draw_connection(*painter, o_pos, mapToScene(m_last_mouse_position), true, reverse);
    }
  }
  painter->restore();
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
  m_last_mouse_position = event->pos();
  if (m_pan_zoom_controller.press(*event)) {
    event->accept();
  } else if (PortItem* port_item = port_item_at(event->pos()); port_item != nullptr) {
    m_tmp_connection_origin = port_item;
    viewport()->update();
    AbstractPort& port = port_item->port;
    if (port.port_type == PortType::Input) {
      InputPort& ip = static_cast<InputPort&>(port);
      if (OutputPort* op = ip.connected_output(); op == nullptr) {
        m_tmp_connection_origin = port_item;
      } else {
        m_former_connection_target = &ip;
        m_about_to_disconnect = &ip;
        m_tmp_connection_origin = m_model->node_item(op->node).port_item(*op);
      }
    } else {
      m_tmp_connection_origin = port_item;
    }
    event->accept();
  } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
    m_node_insert_pos = mapToScene(event->pos());
    Q_EMIT customContextMenuRequested(event->pos());
  } else {
    QGraphicsView::mousePressEvent(event);
  }
}

void NodeView::resizeEvent(QResizeEvent* event)
{
  static const auto s2p = [](const QSize& size) { return QPoint(size.width(), size.height()); };
  const QPointF d = (mapToScene(s2p(event->size())) - mapToScene(s2p(event->oldSize())))/2.0;
  translate(d.x(), d.y());
  reset_scene_rect();
}

void NodeView::mouseDoubleClickEvent(QMouseEvent*)
{
  pan_to_center();
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  if (m_pan_zoom_controller.move(*event)) {
    m_viewport_center = mapToScene(viewport()->rect().center());
    event->accept();
  } else if (m_tmp_connection_origin != nullptr) {
    if (PortItem* port_item = port_item_at(event->pos()); port_item != nullptr
        && model()->can_connect(m_tmp_connection_origin->port, port_item->port))
    {
      m_tmp_connection_target = port_item;
    } else {
      m_tmp_connection_target = nullptr;
    }
    viewport()->update();
  } else {
    m_tmp_connection_target = nullptr;
    if (rubberBandRect().isNull() && event->buttons() == Qt::LeftButton) {
      if (auto* item = itemAt(event->pos()); item != nullptr && item->type() == NodeItem::TYPE) {
        const QPointF current = mapToScene(event->pos());
        const QPointF last = mapToScene(m_last_mouse_position);
        m_model->scene().submit<MoveNodesCommand>(selected_nodes(), current - last);
      }
    }
  }
  m_last_mouse_position = event->pos();
  QGraphicsView::mouseMoveEvent(event);
}

void NodeView::mouseReleaseEvent(QMouseEvent *event)
{
  std::list<std::unique_ptr<Command>> commands;
  if (m_tmp_connection_origin != nullptr && m_tmp_connection_target != nullptr) {
    if (m_model != nullptr) {
      commands.push_back(std::make_unique<ConnectPortsCommand>(m_tmp_connection_origin->port,
                                                               m_tmp_connection_target->port));
      if (m_about_to_disconnect == &m_tmp_connection_origin->port ||
          m_about_to_disconnect == &m_tmp_connection_target->port)
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
      macro = m_model->scene().history().start_macro(tr("Modify Connections"));
    }
    for (auto&& command : commands) {
      m_model->scene().submit(std::move(command));
    }
  }

  m_about_to_disconnect = nullptr;
  m_tmp_connection_origin = nullptr;
  m_tmp_connection_target = nullptr;
  m_pan_zoom_controller.release();
  viewport()->update();
  QGraphicsView::mouseReleaseEvent(event);
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

PortItem* NodeView::port_item_at(const QPoint& pos) const
{
  QGraphicsItem* item = itemAt(pos);
  if (item != nullptr && item->type() == PortItem::TYPE) {
    return static_cast<PortItem*>(item);
  } else {
    return nullptr;
  }
}

//void NodeView::mousePressEvent(QMouseEvent* event)
//{
//  m_aborted = false;
//  m_pzc.move(event->pos());
//  if (m_model != nullptr) {
//    if (preferences().match("shift viewport", *event, true)) {
//      m_pzc.start(PanZoomController::Action::Pan);
//    } else if (preferences().match("zoom viewport", *event, true)) {
//      m_pzc.start(PanZoomController::Action::Zoom);
//    } else {
//      m_pzc.start(PanZoomController::Action::None);
//      const bool extend_selection = event->modifiers() & Qt::ShiftModifier;
//      const bool toggle_selection = event->modifiers() & Qt::ControlModifier;
//      const auto [ node, port ] = select_port_or_node(event->pos() - m_pzc.offset());

//      const auto press_on_port = [this](AbstractPort* port) {
//        if (port->port_type == PortType::Input) {
//          InputPort& ip = static_cast<InputPort&>(*port);
//          if (OutputPort* op = ip.connected_output(); op == nullptr) {
//            m_tmp_connection_origin = &ip;
//          } else {=
//            m_former_connection_target = &ip;
//            m_about_to_disconnect = &ip;
//            m_tmp_connection_origin = op;
//          }
//        } else {
//          m_tmp_connection_origin = port;
//        }
//      };

//      const auto press_on_node = [toggle_selection, extend_selection, this](Node* node) {
//        const bool is_selected = ::contains(m_selection, node);
//        if (toggle_selection) {
//          if (is_selected) {
//            m_selection.erase(node);
//          } else {
//            m_selection.insert(node);
//          }
//        } else {
//          if (!extend_selection && !is_selected) {
//            m_selection.clear();
//          }
//          m_selection.insert(node);
//        }
//        update_scene_selection();
//      };

//      if (node != nullptr || port != nullptr) {
//        if (node != nullptr) {
//          press_on_node(node);
//        } else if (port != nullptr) {
//          press_on_port(port);
//        }
//        if (event->button() == Qt::RightButton) {
//          m_aborted = true;
//          QWidget::mousePressEvent(event);
//        } else {
//          event->accept();
//        }
//      } else if (event->button() == Qt::LeftButton) {
//        m_pzc.rubber_band_visible = true;
//        if (!extend_selection) {
//          m_selection.clear();
//          update_scene_selection();
//        }
//      } else {
//        QWidget::mousePressEvent(event);
//      }
//    }
//    update();
//  } else {
//    QWidget::mousePressEvent(event);
//  }
//}

//void NodeView::mouseMoveEvent(QMouseEvent* event)
//{
//  QToolTip::hideText();
//  if (!m_aborted && event->buttons() != 0) {
//    if (m_pzc.move(event->pos())) {
//    } else if (m_pzc.rubber_band_visible) {
//      m_nodes_in_rubberband = this->nodes(m_pzc.unit_rubber_band());
//    } else if (m_tmp_connection_origin == nullptr && m_model != nullptr) {
//      if (event->buttons() & Qt::LeftButton) {
//        if (const QPointF e = m_pzc.unit_d(); e.manhattanLength() > 0 && !m_selection.empty()) {
//          m_model->scene().submit<MoveNodesCommand>(m_selection, e);
//        }
//      }
//    } else if (m_tmp_connection_origin != nullptr && m_model != nullptr) {
//      AbstractPort* port = this->port_at(m_model->ports(), event->pos() - m_pzc.offset());
//      if (port != nullptr && m_model->can_connect(*port, *m_tmp_connection_origin)) {
//        m_tmp_connection_target = port;
//      } else {
//        m_tmp_connection_target = nullptr;
//      }
//    }
//    update();
//  }
//}

//void NodeView::mouseReleaseEvent(QMouseEvent*)
//{
//  std::list<std::unique_ptr<Command>> commands;
//  if (m_tmp_connection_origin != nullptr && m_tmp_connection_target != nullptr) {
//    if (m_model != nullptr) {
//      commands.push_back(std::make_unique<ConnectPortsCommand>(*m_tmp_connection_origin,
//                                                               *m_tmp_connection_target));
//      if (m_about_to_disconnect == m_tmp_connection_origin
//          || m_about_to_disconnect == m_tmp_connection_target)
//      {
//        m_about_to_disconnect = nullptr;
//      }
//    }
//  }
//  if (m_about_to_disconnect != nullptr) {
//    commands.push_back(std::make_unique<DisconnectPortsCommand>(*m_about_to_disconnect));
//  }

//  if (m_pzc.rubber_band_visible) {
//    m_pzc.rubber_band_visible = false;
//    m_selection.insert(m_nodes_in_rubberband.begin(), m_nodes_in_rubberband.end());
//    update_scene_selection();
//    m_nodes_in_rubberband.clear();
//  }

//  {
//    std::unique_ptr<Macro> macro;
//    if (commands.size() > 1) {
//      macro = m_model->scene().history().start_macro(tr("Modify Connections"));
//    }
//    for (auto&& command : commands) {
//      m_model->scene().submit(std::move(command));
//    }
//  }

//  m_former_connection_target = nullptr;
//  m_tmp_connection_origin = nullptr;
//  m_tmp_connection_target = nullptr;
//  m_about_to_disconnect = nullptr;
//  m_aborted = false;
//  m_pzc.end();
//  update();
//}

//void NodeView::dragEnterEvent(QDragEnterEvent* event)
//{
//  event->setDropAction(Qt::LinkAction);
//  if (can_drop(*event)) {
//    event->accept();
//  } else {
//    event->ignore();
//  }
//}

//void NodeView::dropEvent(QDropEvent* event)
//{
//  event->setDropAction(Qt::LinkAction);
//  if (can_drop(*event)) {
//    const auto& mime_data = *event->mimeData();
//    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
//    const auto items = property_owner_mime_data.items(m_droppable_kinds);

//    std::vector<std::unique_ptr<Node>> nodes;
//    nodes.reserve(items.size());
//    Scene& scene = this->scene()->scene();
//    QPointF insert_pos = get_insert_position(event->pos());
//    for (AbstractPropertyOwner* item : items) {
//      auto node = Node::make(ReferenceNode::TYPE, *this->scene());
//      auto* const property = node->property(ReferenceNode::REFERENCE_PROPERTY_KEY);
//      property->set(item);
//      const QSizeF size = node_geometry(*node).size();
//      insert_pos -= QPointF(size.width(), size.height()) / 2.0;
//      node->set_pos(insert_pos);
//      nodes.push_back(std::move(node));
//    }
//    scene.submit<AddNodesCommand>(*this->scene(), std::move(nodes));
//  } else {
//    event->ignore();
//  }
//}

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
    const auto selection = ::transform<Node*, std::vector>(selected_nodes());
    m_model->scene().submit<RemoveNodesCommand>(*m_model, selection);
  }
}

void NodeView::draw_connection(QPainter& painter, const QPointF& in, const QPointF& out,
                               bool is_floating, bool reverse) const
{
  if (reverse) {
    draw_connection(painter, out, in, is_floating, false);
  } else {
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
    pen.setWidth(2.0);
    pen.setColor(ui_color(*this, "NodeView", is_floating ? "floating-connection"
                                                         : "fixed-connection"));
    painter.setPen(pen);
    painter.drawPath(path);
    painter.restore();
  }
}

void NodeView::populate_context_menu(QMenu& menu) const
{
  if (const auto selection = selected_nodes(); selection.size() == 1) {
    (**selection.begin()).populate_menu(menu);
  }
}

void NodeView::pan_to_center()
{
  if (m_model != nullptr) {
    const auto nodes = m_model->nodes();
    const auto f = [](const QPointF& p, const Node* n) { return p + n->pos(); };
    const double n = nodes.size();
    const auto center = std::accumulate(nodes.begin(), nodes.end(), QPointF(), f) / std::max(1.0, n);
    reset_scene_rect();
    const auto d = mapToScene(viewport()->rect().center()) - center;
    translate(d.x(), d.y());
    reset_scene_rect();
    update();
  }
}

}  // namespace omm
