#include "commands/nodecommand.h"

#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/port.h"
#include <utility>

namespace
{

std::map<omm::nodes::Node*, QPointF> collect_old_positions(const std::set<omm::nodes::Node*>& nodes)
{
  std::map<omm::nodes::Node*, QPointF> map;
  for (omm::nodes::Node* node : nodes) {
    map.insert({node, node->pos()});
  }
  return map;
}

}  // namespace

namespace omm
{
ConnectionCommand::ConnectionCommand(const QString& label, nodes::AbstractPort& a, nodes::AbstractPort& b)
    : ConnectionCommand(label,
                        dynamic_cast<nodes::OutputPort&>(b.port_type == nodes::PortType::Input ? a : b),
                        dynamic_cast<nodes::InputPort&>(a.port_type == nodes::PortType::Input ? a : b))
{
  // require exactly one input and one output.
  assert(a.port_type != b.port_type);
}

ConnectionCommand::ConnectionCommand(const QString& label, nodes::OutputPort& out, nodes::InputPort& in)
    : Command(label), m_source_node(out.node), m_output_index(out.index), m_target_node(in.node),
      m_input_index(in.index)
{
}

void ConnectionCommand::connect()
{
  input_port().connect(&output_port());
}

void ConnectionCommand::disconnect()
{
  input_port().connect(nullptr);
}

nodes::InputPort& ConnectionCommand::input_port() const
{
  return *m_target_node.find_port<nodes::InputPort>(m_input_index);
}

nodes::OutputPort& ConnectionCommand::output_port() const
{
  return *m_source_node.find_port<nodes::OutputPort>(m_output_index);
}

ConnectPortsCommand::ConnectPortsCommand(nodes::AbstractPort& a, nodes::AbstractPort& b)
    : ConnectionCommand(QObject::tr("Connect Ports"), a, b)
{
}

DisconnectPortsCommand::DisconnectPortsCommand(nodes::InputPort& port)
    : ConnectionCommand(QObject::tr("Disconnect Ports"), port, *port.connected_output())
{
}

NodeCommand::NodeCommand(const QString& label,
                         nodes::NodeModel& model,
                         const std::vector<nodes::Node*>& refs,
                         std::vector<std::unique_ptr<nodes::Node>> owns)
    : Command(label), m_refs(refs), m_owns(std::move(owns)), m_model(model)
{
  if (m_refs.empty()) {
    for (const auto& node : m_owns) {
      m_refs.push_back(node.get());
    }
  }

#ifndef NDEBUG
  if (!m_owns.empty()) {
    assert(m_refs.size() == m_owns.size());
    for (std::size_t i = 0; i < m_refs.size(); ++i) {
      assert(m_refs.at(i) == m_owns.at(i).get());
    }
  }
#endif
}

void NodeCommand::remove()
{
  {
    nodes::NodeModel::TopologyChangeSignalBlocker blocker(m_model);
    m_owns.reserve(m_refs.size());
    for (nodes::Node* node : m_refs) {
      const auto break_connection = [this](nodes::InputPort& ip) {
        m_destroyed_connections.emplace_back(ip);
        m_destroyed_connections.back().redo();
      };
      for (nodes::InputPort* ip : node->ports<nodes::InputPort>()) {
        if (nodes::OutputPort* op = ip->connected_output(); op != nullptr) {
          if (!::contains(m_refs, &op->node)) {
            break_connection(*ip);
          }
        }
      }
      for (nodes::OutputPort* op : node->ports<nodes::OutputPort>()) {
        for (nodes::InputPort* ip : op->connected_inputs()) {
          if (!::contains(m_refs, &ip->node)) {
            break_connection(*ip);
          }
        }
      }
      m_owns.push_back(m_model.extract_node(*node));
    }
  }
  m_model.emit_topology_changed();
}

void NodeCommand::add()
{
  {
    nodes::NodeModel::TopologyChangeSignalBlocker blocker(m_model);
    for (auto&& node : m_owns) {
      m_model.add_node(std::move(node));
    }
    for (auto it = m_destroyed_connections.rbegin(); it != m_destroyed_connections.rend(); ++it) {
      it->undo();
    }
    m_destroyed_connections.clear();
    m_owns.clear();
  }
  m_model.emit_topology_changed();
}

RemoveNodesCommand::RemoveNodesCommand(nodes::NodeModel& model, const std::vector<nodes::Node*>& nodes)
    : NodeCommand(QObject::tr("Remove Nodes"), model, nodes, {})
{
}

AddNodesCommand::AddNodesCommand(nodes::NodeModel& model, std::vector<std::unique_ptr<nodes::Node>> nodes)
    : NodeCommand(QObject::tr("Add Nodes"), model, {}, std::move(nodes))
{
}

MoveNodesCommand::MoveNodesCommand(const std::set<nodes::Node*>& nodes, const QPointF& direction)
    : Command(QObject::tr("Move Nodes")), m_old_positions(collect_old_positions(nodes)),
      m_direction(direction)
{
}

void MoveNodesCommand::undo()
{
  for (auto&& [node, old_pos] : m_old_positions) {
    node->set_pos(old_pos);
  }
}

void MoveNodesCommand::redo()
{
  for (auto&& [node, old_pos] : m_old_positions) {
    node->set_pos(old_pos + m_direction);
  }
}

bool MoveNodesCommand::mergeWith(const QUndoCommand* command)
{
  const auto& mn_command = dynamic_cast<const MoveNodesCommand&>(*command);
  if (::get_keys(mn_command.m_old_positions) == ::get_keys(m_old_positions)) {
    m_direction += mn_command.m_direction;
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
