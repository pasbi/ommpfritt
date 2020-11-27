#include "commands/nodecommand.h"
#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/port.h"

namespace
{
std::map<omm::Node*, QPointF> collect_old_positions(const std::set<omm::Node*>& nodes)
{
  std::map<omm::Node*, QPointF> map;
  for (omm::Node* node : nodes) {
    map.insert({node, node->pos()});
  }
  return map;
}

}  // namespace

namespace omm
{
ConnectionCommand::ConnectionCommand(const QString& label, AbstractPort& a, AbstractPort& b)
    : ConnectionCommand(label,
                        dynamic_cast<OutputPort&>(b.port_type == PortType::Input ? a : b),
                        dynamic_cast<InputPort&>(a.port_type == PortType::Input ? a : b))
{
  // require exactly one input and one output.
  assert(a.port_type != b.port_type);
}

ConnectionCommand::ConnectionCommand(const QString& label, OutputPort& out, InputPort& in)
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

InputPort& ConnectionCommand::input_port() const
{
  return *m_target_node.find_port<InputPort>(m_input_index);
}

OutputPort& ConnectionCommand::output_port() const
{
  return *m_source_node.find_port<OutputPort>(m_output_index);
}

ConnectPortsCommand::ConnectPortsCommand(AbstractPort& a, AbstractPort& b)
    : ConnectionCommand(QObject::tr("Connect Ports"), a, b)
{
}

DisconnectPortsCommand::DisconnectPortsCommand(InputPort& port)
    : ConnectionCommand(QObject::tr("Disconnect Ports"), port, *port.connected_output())
{
}

NodeCommand::NodeCommand(const QString& label,
                         NodeModel& model,
                         std::vector<Node*> refs,
                         std::vector<std::unique_ptr<Node>> owns)
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
    NodeModel::TopologyChangeSignalBlocker blocker(m_model);
    m_owns.reserve(m_refs.size());
    for (Node* node : m_refs) {
      const auto break_connection = [this](InputPort& ip) {
        m_destroyed_connections.emplace_back(ip);
        m_destroyed_connections.back().redo();
      };
      for (InputPort* ip : node->ports<InputPort>()) {
        if (OutputPort* op = ip->connected_output(); op != nullptr) {
          if (!::contains(m_refs, &op->node)) {
            break_connection(*ip);
          }
        }
      }
      for (OutputPort* op : node->ports<OutputPort>()) {
        for (InputPort* ip : op->connected_inputs()) {
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
    NodeModel::TopologyChangeSignalBlocker blocker(m_model);
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

RemoveNodesCommand::RemoveNodesCommand(NodeModel& model, std::vector<Node*> nodes)
    : NodeCommand(QObject::tr("Remove Nodes"), model, nodes, {})
{
}

AddNodesCommand::AddNodesCommand(NodeModel& model, std::vector<std::unique_ptr<Node>> nodes)
    : NodeCommand(QObject::tr("Add Nodes"), model, {}, std::move(nodes))
{
}

MoveNodesCommand::MoveNodesCommand(const std::set<Node*>& nodes, const QPointF& direction)
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
  if (::same_keys(mn_command.m_old_positions, m_old_positions)) {
    m_direction += mn_command.m_direction;
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
