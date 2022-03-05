#include "managers/nodemanager/nodeview.h"
#include "commands/nodecommand.h"
#include "main/application.h"
#include "managers/nodemanager/nodeitem.h"
#include "managers/nodemanager/nodemimedata.h"
#include "managers/nodemanager/nodescene.h"
#include "managers/nodemanager/portitem.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/referencenode.h"
#include "preferences/preferences.h"
#include "preferences/uicolors.h"
#include "properties/referenceproperty.h"
#include "propertywidgets/propertywidget.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/propertyownermimedata.h"
#include "scene/scene.h"
#include "removeif.h"
#include <QApplication>
#include <QClipboard>
#include <QGraphicsItem>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace
{
omm::PortItem* get_port_item(omm::NodeScene& scene, omm::nodes::AbstractPort& port)
{
  omm::NodeItem& node_item = scene.node_item(port.node);
  return node_item.port_item(port);
}

std::vector<omm::AbstractPropertyOwner*> items(const QDropEvent& event)
{
  const QMimeData& mime_data = *event.mimeData();
  if (mime_data.hasFormat(omm::PropertyOwnerMimeData::MIME_TYPE)) {
    const auto* pomd = qobject_cast<const omm::PropertyOwnerMimeData*>(&mime_data);
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

std::vector<double> linspace(double left, double right, double step)
{
  const auto mod = [step](const double v) { return std::fmod(std::fmod(v, step) + step, step); };

  left = left - mod(left);
  right = right - mod(right) + step;

  const std::size_t n = (right - left) / step;
  std::vector<double> linspace;
  linspace.reserve(n + 1);
  for (std::size_t i = 0; i < n + 1; ++i) {
    const double v = left + static_cast<double>(i) * step;
    linspace.push_back(v);
  }
  return linspace;
}

}  // namespace

namespace omm
{

NodeView::NodeView(QWidget* parent) : QGraphicsView(parent), m_pan_zoom_controller(*this)
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
}

NodeView::~NodeView() = default;

void NodeView::set_model(nodes::NodeModel* model)
{
  auto* const current_model = m_node_scene == nullptr ? nullptr : m_node_scene->model();
  if (current_model != nullptr) {
    QObject::disconnect(m_view_scene_connection);
  }
  if (model == nullptr) {
    m_node_scene = nullptr;
    setScene(nullptr);
  } else {
    m_node_scene = std::make_unique<NodeScene>(*model->scene());
    m_node_scene->set_model(model);
    m_view_scene_connection = connect(model,
                                      &nodes::NodeModel::topology_changed,
                                      m_node_scene.get(),
                                      [s = m_node_scene.get()]() { s->update(); });
    setScene(m_node_scene.get());
    const QRectF scene_rect = viewport()->rect();
    setSceneRect(scene_rect);
    pan_to_center();
  }
}

nodes::NodeModel* NodeView::model() const
{
  if (m_node_scene == nullptr) {
    return nullptr;
  } else {
    return m_node_scene->model();
  }
}

bool NodeView::accepts_paste(const QMimeData& mime_data) const
{
  if (auto* model = this->model(); model != nullptr) {
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

std::set<nodes::Node*> NodeView::selected_nodes() const
{
  if (m_node_scene == nullptr) {
    return {};
  } else {
    return m_node_scene->selected_nodes();
  }
}

void NodeView::copy_to_clipboard() const
{
  if (auto* const model = this->model(); model != nullptr) {
    auto mime_data = std::make_unique<NodeMimeData>(model->language(), selected_nodes());
    QApplication::clipboard()->setMimeData(mime_data.release());
  }
}

void NodeView::paste_from_clipboard()
{
  const QMimeData& mime_data = *QApplication::clipboard()->mimeData();
  if (accepts_paste(mime_data)) {
    auto& model = *this->model();
    auto blocker = std::make_unique<nodes::NodeModel::TopologyChangeSignalBlocker>(model);
    const auto nodes
        = util::transform<std::vector>(dynamic_cast<const NodeMimeData&>(mime_data).nodes());

    std::map<const nodes::Node*, nodes::Node*> copy_map;
    const auto copyable_nodes = util::remove_if(nodes, [](const auto& node) {
      return !node->copyable();
    });
    const auto make_copy = [&model, &copy_map](auto* const node) {
      auto clone = node->clone(model);
      copy_map[node] = clone.get();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      return clone;
    };
    auto copies = util::transform<std::vector>(copyable_nodes, make_copy);

    {  // set position
      const auto old_center
          = std::accumulate(nodes.begin(),
                            nodes.end(),
                            QPointF(),
                            [](const QPointF& p, const auto* const node) { return node->pos() + p; })
            / nodes.size();
      const auto diff = mapToScene(mapFromGlobal(QCursor::pos())) - old_center;
      for (auto& node : copies) {
        node->set_pos(node->pos() + diff);
      }
    }

    if (!copies.empty()) {
      {
        Scene& scene = *model.scene();
        auto macro = scene.history().start_macro(tr("Copy Nodes"));

        {  // restore connections
          for (auto&& [o_target, c_target] : copy_map) {
            for (const auto* const o_input : o_target->ports<nodes::InputPort>()) {
              if (const auto* const o_output = o_input->connected_output(); o_output != nullptr) {
                const auto& o_source = o_output->node;
                if (copy_map.contains(&o_source)) {
                  const auto& c_source = *copy_map.at(&o_source);
                  auto& c_output = *c_source.find_port<nodes::OutputPort>(o_output->index);
                  auto& c_input = *c_target->find_port<nodes::InputPort>(o_input->index);
                  scene.submit<ConnectPortsCommand>(c_output, c_input);
                }
              }
            }
          }
        }

        // insert nodes
        auto references = util::transform(copies, [](const auto& node_uptr) {
          return node_uptr.get();
        });
        scene.submit<AddNodesCommand>(model, std::move(copies));
        m_node_scene->clearSelection();
        for (auto* const node : references) {
          m_node_scene->node_item(*node).setSelected(true);
        }
      }
      blocker.reset();
      model.emit_topology_changed();
    }
  }
}

void NodeView::draw_status_bar(QPainter& painter)
{
  if (const auto* const model = this->model(); model != nullptr) {
    if (QString error = model->error(); !error.isEmpty()) {
      painter.save();
      painter.resetTransform();
      const QRect rect = viewport()->rect();
      const QRect status_bar(rect.bottomLeft() - QPoint(0, 20), rect.bottomRight());
      painter.fillRect(status_bar, ui_color(*this, "NodeView", "statusbar-bg"));
      painter.drawText(status_bar, Qt::AlignVCenter | Qt::AlignLeft, error);
      painter.restore();
    }
  }
}

void NodeView::drawForeground(QPainter* painter, const QRectF&)
{
  static constexpr auto reverse_connection = [](const PortItem& origin) {
    return origin.port.port_type == nodes::PortType::Output;
  };
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setTransform(viewportTransform());
  if (auto* const model = this->model(); model != nullptr) {
    for (auto* const node : model->nodes()) {
      for (auto* const ip : node->ports<nodes::InputPort>()) {
        if (auto* const op = ip->connected_output(); op != nullptr) {
          const auto* op_item = get_port_item(*m_node_scene, *op);
          const auto* ip_item = get_port_item(*m_node_scene, *ip);
          if (op_item != nullptr && ip_item != nullptr) {
            draw_connection(*painter, ip_item->scenePos(), op_item->scenePos(), false, false);
          }
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

  draw_status_bar(*painter);
}

void NodeView::drawBackground(QPainter* painter, const QRectF&)
{
  painter->save();
  const auto top_left = sceneRect().topLeft();
  const auto bottom_right = sceneRect().bottomRight();
  const double scale = std::min((bottom_right.x() - top_left.x()) / viewport()->width(),
                                (bottom_right.y() - top_left.y()) / viewport()->height());

  const double step = scale > 1.2 ? 100.0 : 50.0;

  if (const auto* const model = this->model(); model != nullptr && !model->error().isEmpty()) {
    painter->fillRect(sceneRect(), ui_color(*this, "NodeView", "canvas-bg-invalid"));
  } else {
    painter->fillRect(sceneRect(), ui_color(*this, "NodeView", "canvas-bg-valid"));
  }

  static constexpr double GRID_LINE_WIDTH = 0.5;
  QPen pen;
  pen.setCosmetic(true);
  pen.setWidthF(GRID_LINE_WIDTH);
  pen.setColor(ui_color(*this, "NodeView", "grid-color"));
  painter->setPen(pen);

  for (double y : linspace(top_left.y(), bottom_right.y(), step)) {
    painter->drawLine(QPointF{top_left.x(), y}, QPointF{bottom_right.x(), y});
  }
  for (double x : linspace(top_left.x(), bottom_right.x(), step)) {
    painter->drawLine(QPointF{x, top_left.y()}, QPointF{x, bottom_right.y()});
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
  } else if (auto* port_item = item_at<PortItem>(event->pos()); port_item != nullptr) {
    m_tmp_connection_origin = port_item;
    viewport()->update();
    auto& port = port_item->port;
    if (port.port_type == nodes::PortType::Input) {
      auto& ip = dynamic_cast<nodes::InputPort&>(port);
      if (auto* const op = ip.connected_output(); op == nullptr) {
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
  static const auto rect_to_polygon = [](const QRectF& rect) {
    QPolygonF polygon(rect);
    // it should be closed.
    assert(polygon.front() == polygon.back());
    polygon.removeLast();
    return polygon;
  };

  const auto scene_center = [this](const QSizeF& viewport_size) {
    QTransform t;
    QTransform::quadToQuad(rect_to_polygon({QPointF(), viewport_size}),
                           rect_to_polygon(sceneRect()),
                           t);
    return t.map(QPointF{viewport_size.width(), viewport_size.height()} / 2.0);
  };

  const auto old_scene_pos = scene_center(event->oldSize());
  reset_scene_rect();
  const QPointF d = old_scene_pos - scene_center(event->size());
  setSceneRect(sceneRect().translated(d));
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
  if (auto* model = this->model(); model != nullptr && can_drop(*event)) {
    QPointF pos = mapToScene(event->pos());
    auto nodes = util::transform(::items(*event), [&pos, model](auto* apo) -> std::unique_ptr<omm::nodes::Node> {
      auto reference_node = std::make_unique<nodes::ReferenceNode>(*model);
      reference_node->property(nodes::ReferenceNode::REFERENCE_PROPERTY_KEY)->set(apo);
      reference_node->set_pos(pos);
      static constexpr int OFFSET = 50;
      pos += QPointF(OFFSET, OFFSET);
      return reference_node;
    });
    model->scene()->submit<AddNodesCommand>(*model, std::move(nodes));
  } else {
    QGraphicsView::dropEvent(event);
  }
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
  if (m_pan_zoom_controller.move(*event)) {
    event->accept();
  } else if (m_tmp_connection_origin != nullptr) {
    if (auto* port_item = item_at<PortItem>(event->pos());
        port_item != nullptr
        && model()->can_connect(m_tmp_connection_origin->port, port_item->port)) {
      m_tmp_connection_target = port_item;
    } else {
      m_tmp_connection_target = nullptr;
    }
    viewport()->update();
  } else if (auto* const model = this->model(); model != nullptr) {
    m_tmp_connection_target = nullptr;
    if (rubberBandRect().isNull() && event->buttons() == Qt::LeftButton) {
      if (auto* item = itemAt(event->pos()); item != nullptr && item->type() == NodeItem::TYPE) {
        const QPointF current = mapToScene(event->pos());
        const QPointF last = mapToScene(m_last_mouse_position);
        model->scene()->submit<MoveNodesCommand>(selected_nodes(), current - last);
      }
    }
  }
  m_last_mouse_position = event->pos();
  QGraphicsView::mouseMoveEvent(event);
}

void NodeView::mouseReleaseEvent(QMouseEvent* event)
{
  std::list<std::unique_ptr<Command>> commands;
  const auto maybe_disconnect = [&commands](auto& port) {
    if (port.port_type == nodes::PortType::Input) {
      auto& ip = dynamic_cast<nodes::InputPort&>(port);
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
      if (m_about_to_disconnect == &m_tmp_connection_origin->port
          || m_about_to_disconnect == &m_tmp_connection_target->port) {
        m_about_to_disconnect = nullptr;
      }
    }
  }
  if (m_about_to_disconnect != nullptr) {
    commands.push_back(std::make_unique<DisconnectPortsCommand>(*m_about_to_disconnect));
  }

  if (auto* model = this->model(); model != nullptr) {
    std::unique_ptr<Macro> macro;
    if (commands.size() > 1) {
      macro = model->scene()->history().start_macro(tr("Modify Connections"));
    }
    for (auto&& command : commands) {
      model->scene()->submit(std::move(command));
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
    return !::items(event).empty();
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

void NodeView::remove_selection() const
{
  if (auto* model = this->model(); model != nullptr) {
    auto selection = util::remove_if(util::transform<std::vector>(selected_nodes()), [](const auto* const n) {
      return n->type() == nodes::FragmentNode::TYPE;
    });
    model->scene()->submit<RemoveNodesCommand>(*model, selection);
  }
}

void NodeView::draw_connection(QPainter& painter,
                               const QPointF& in,
                               const QPointF& out,
                               bool is_floating,
                               bool reverse) const
{
  if (reverse) {
    draw_connection(painter, out, in, is_floating, false);
  } else {
#define INTERPOLATE_CONNECTION_AWAY_FROM_NODE
#if defined(INTERPOLATE_CONNECTION_CORRECT_DIRECTION)
    const QPointF c1((2.0 * in.x() + 1.0 * out.x()) / 3.0, in.y());
    const QPointF c2((1.0 * in.x() + 2.0 * out.x()) / 3.0, out.y());
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
    static constexpr double CONNECTION_PEN_WIDTH = 2.0;
    pen.setWidthF(CONNECTION_PEN_WIDTH);
    pen.setColor(
        ui_color(*this, "NodeView", is_floating ? "floating-connection" : "fixed-connection"));
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
  if (auto* model = this->model(); model != nullptr) {
    const auto nodes = model->nodes();
    const auto f = [](const auto& accu, const auto* const node) { return accu + node->pos(); };
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
