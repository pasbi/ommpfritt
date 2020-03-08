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
#include "managers/nodemanager/nodescene.h"
#include "managers/nodemanager/nodes/fragmentnode.h"

namespace
{

omm::PortItem* get_port_item(omm::NodeScene& scene, omm::AbstractPort& port)
{
  omm::NodeItem& node_item = scene.node_item(port.node);
  return node_item.port_item(port);
}

std::vector<omm::AbstractPropertyOwner*> items(const QDropEvent& event)
{
  const QMimeData& mime_data = *event.mimeData();
  if (mime_data.hasFormat(omm::PropertyOwnerMimeData::MIME_TYPE)) {
    const auto pomd = qobject_cast<const omm::PropertyOwnerMimeData*>(&mime_data);
    if (pomd != nullptr) {
      return pomd->items();
    }
  }
  return {};
}

QGraphicsItem* root(QGraphicsItem* item)
{
  QGraphicsItem* parent = nullptr;
  while (true) {
    if (parent = item->parentItem(); parent != nullptr) {
      item = parent;
    } else {
      return item;
    }
  }
  Q_UNREACHABLE();
  return nullptr;
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
  if (m_node_scene == nullptr) {
    m_node_scene = std::make_unique<NodeScene>(model->scene());
  } else {
    assert(&m_node_scene->scene == &model->scene());
  }
  if (auto* model = m_node_scene->model(); model != nullptr) {
    disconnect(model, SIGNAL(topology_changed()), m_node_scene.get(), SLOT(update()));
  }
  m_node_scene->set_model(model);
  if (auto* model = m_node_scene->model(); model != nullptr) {
    connect(model, SIGNAL(topology_changed()), m_node_scene.get(), SLOT(update()));
  }
  setScene(m_node_scene.get());
  const QRectF scene_rect = viewport()->rect();
  setSceneRect(scene_rect);
  pan_to_center();
}

NodeModel* NodeView::model() const
{
  if (m_node_scene == nullptr) {
    return nullptr;
  } else {
    return m_node_scene->model();
  }
}

bool NodeView::accepts_paste(const QMimeData &mime_data) const
{
  if (auto model = this->model(); model != nullptr) {
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

std::set<Node*> NodeView::selected_nodes() const
{
  if (m_node_scene == nullptr) {
    return {};
  } else {
    return m_node_scene->selected_nodes();
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
    auto blocker = std::make_unique<NodeModel::TopologyChangeSignalBlocker>(model);
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
      {
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
        auto references = ::transform<Node*>(copies, [](const std::unique_ptr<Node>& node) {
          return node.get();
        });
        scene.submit<AddNodesCommand>(model, std::move(copies));
        m_node_scene->clearSelection();
        for (Node* node : references) {
          m_node_scene->node_item(*node).setSelected(true);
        }
      }
      blocker.reset();
      model.emit_topology_changed();
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
          const QPointF input_port_item_pos = get_port_item(*m_node_scene, *ip)->scenePos();
          const QPointF output_port_item_pos = get_port_item(*m_node_scene, *op)->scenePos();
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

void NodeView::drawBackground(QPainter* painter, const QRectF&)
{
  painter->save();
  painter->resetTransform();
  if (const NodeModel* model = this->model(); model != nullptr) {
    if (QString error = model->error(); !error.isEmpty()) {
      const QRect rect = viewport()->rect();
      painter->fillRect(rect, QColor(255, 0, 0, 40));

      const QRect status_bar(rect.bottomLeft() - QPoint(0, 20), rect.bottomRight());
      painter->fillRect(status_bar, QColor(0, 0, 0, 140));
      painter->drawText(status_bar, Qt::AlignVCenter | Qt::AlignLeft, error);
    }
  } else {
    painter->fillRect(rect(), Qt::yellow);
  }
  painter->restore();
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
  QGraphicsScene* scene = this->scene();
  if (scene == nullptr) {
    // do nothing if no scene is loaded
    return;
  }

  Q_EMIT scene->selectionChanged();
  m_last_mouse_position = event->pos();
  if (m_pan_zoom_controller.press(*event)) {
    event->accept();
  } else if (PortItem* port_item = item_at<PortItem>(event->pos()); port_item != nullptr) {
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
        m_tmp_connection_origin = m_node_scene->node_item(op->node).port_item(*op);
      }
    } else {
      m_tmp_connection_origin = port_item;
    }
    event->accept();
  } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
    if (auto* item = itemAt(event->pos()); item != nullptr) {
      if (auto* root = ::root(item); root->type() == NodeItem::TYPE && !root->isSelected()) {
        scene->clearSelection();
        root->setSelected(true);
      }
    } else {
      scene->clearSelection();
    }
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

void NodeView::dragMoveEvent(QDragMoveEvent* event)
{
  if (model() != nullptr && can_drop(*event)) {
    event->accept();
  } else {
    QGraphicsView::dragMoveEvent(event);
  }
}

void NodeView::dropEvent(QDropEvent* event)
{
  if (auto model = this->model(); model != nullptr && can_drop(*event)) {
    QPointF pos = mapToScene(event->pos());
    auto nodes = ::transform<std::unique_ptr<Node>>(::items(*event), [&pos, model](auto* apo) {
      auto reference_node = std::make_unique<ReferenceNode>(*model);
      reference_node->property(ReferenceNode::REFERENCE_PROPERTY_KEY)->set(apo);
      reference_node->set_pos(pos);
      pos += QPointF(50, 50);
      return reference_node;
    });
    model->scene().submit<AddNodesCommand>(*model, std::move(nodes));
  } else {
    QGraphicsView::dropEvent(event);
  }
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  if (m_pan_zoom_controller.move(*event)) {
    m_viewport_center = mapToScene(viewport()->rect().center());
    event->accept();
  } else if (m_tmp_connection_origin != nullptr) {
    if (PortItem* port_item = item_at<PortItem>(event->pos()); port_item != nullptr
        && model()->can_connect(m_tmp_connection_origin->port, port_item->port))
    {
      m_tmp_connection_target = port_item;
    } else {
      m_tmp_connection_target = nullptr;
    }
    viewport()->update();
  } else if (NodeModel* model = this->model(); model != nullptr) {
    m_tmp_connection_target = nullptr;
    if (rubberBandRect().isNull() && event->buttons() == Qt::LeftButton) {
      if (auto* item = itemAt(event->pos()); item != nullptr && item->type() == NodeItem::TYPE) {
        const QPointF current = mapToScene(event->pos());
        const QPointF last = mapToScene(m_last_mouse_position);
        model->scene().submit<MoveNodesCommand>(selected_nodes(), current - last);
      }
    }
  }
  m_last_mouse_position = event->pos();
  QGraphicsView::mouseMoveEvent(event);
}

void NodeView::mouseReleaseEvent(QMouseEvent *event)
{

  std::list<std::unique_ptr<Command>> commands;
  const auto maybe_disconnect = [&commands](AbstractPort& port) {
    if (port.port_type == PortType::Input) {
      auto& ip = static_cast<InputPort&>(port);
      if (ip.is_connected()) {
        commands.push_back(std::make_unique<DisconnectPortsCommand>(ip));
      }
    }
  };
  if (m_tmp_connection_origin != nullptr && m_tmp_connection_target != nullptr) {
    if (model() != nullptr) {
      maybe_disconnect(m_tmp_connection_origin->port);
      maybe_disconnect(m_tmp_connection_target->port);
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

  if (auto model = this->model(); model != nullptr) {
    std::unique_ptr<Macro> macro;
    if (commands.size() > 1) {
      macro = model->scene().history().start_macro(tr("Modify Connections"));
    }
    for (auto&& command : commands) {
      model->scene().submit(std::move(command));
    }
  }

  m_about_to_disconnect = nullptr;
  m_tmp_connection_origin = nullptr;
  m_tmp_connection_target = nullptr;
  m_pan_zoom_controller.release();
  viewport()->update();
  QGraphicsView::mouseReleaseEvent(event);
}

bool NodeView::can_drop(const QDropEvent& event)
{
  if (itemAt(event.pos()) != nullptr) {
    return false;  // drop on items is handled elsewhere
  } else {
    return ::items(event).size() > 0;
  }
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
  static const auto can_remove = [](const Node* n) { return n->type() != FragmentNode::TYPE; };
  if (auto model = this->model(); model != nullptr) {
    auto selection = ::filter_if(::transform<Node*, std::vector>(selected_nodes()), can_remove);
    model->scene().submit<RemoveNodesCommand>(*model, selection);
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
  if (auto model = this->model(); model != nullptr) {
    const auto nodes = model->nodes();
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
