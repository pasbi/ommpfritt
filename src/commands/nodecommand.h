#pragma once

#include "commands/command.h"
#include <QPointF>
#include <memory>
#include <set>

namespace omm::nodes
{

class NodeModel;
class Node;
class InputPort;
class OutputPort;
class AbstractPort;

}  // namespace omm::nodes

namespace omm
{

class ConnectionCommand : public Command
{
protected:
  ConnectionCommand(const QString& label, nodes::AbstractPort& a, nodes::AbstractPort& b);
  ConnectionCommand(const QString& label, nodes::OutputPort& out, nodes::InputPort& in);
  void connect();
  void disconnect();

private:
  nodes::Node& m_source_node;
  const std::size_t m_output_index;
  nodes::Node& m_target_node;
  const std::size_t m_input_index;

  [[nodiscard]] nodes::InputPort& input_port() const;
  [[nodiscard]] nodes::OutputPort& output_port() const;
};

class ConnectPortsCommand : public ConnectionCommand
{
public:
  ConnectPortsCommand(nodes::AbstractPort& a, nodes::AbstractPort& b);
  void undo() override
  {
    disconnect();
  }
  void redo() override
  {
    connect();
  }
};

class DisconnectPortsCommand : public ConnectionCommand
{
public:
  DisconnectPortsCommand(nodes::InputPort& port);
  void undo() override
  {
    connect();
  }
  void redo() override
  {
    disconnect();
  }
};

class NodeCommand : public Command
{
protected:
  NodeCommand(const QString& label,
              nodes::NodeModel& model,
              const std::vector<nodes::Node*>& refs,
              std::vector<std::unique_ptr<nodes::Node>> owns);
  void remove();
  void add();

private:
  std::vector<nodes::Node*> m_refs;
  std::vector<std::unique_ptr<nodes::Node>> m_owns;
  nodes::NodeModel& m_model;
  std::list<DisconnectPortsCommand> m_destroyed_connections;
};

class RemoveNodesCommand : public NodeCommand
{
public:
  RemoveNodesCommand(nodes::NodeModel& model, const std::vector<nodes::Node*>& nodes);
  void undo() override
  {
    add();
  }
  void redo() override
  {
    remove();
  }
};

class AddNodesCommand : public NodeCommand
{
public:
  AddNodesCommand(nodes::NodeModel& model, std::vector<std::unique_ptr<nodes::Node>> nodes);
  void undo() override
  {
    remove();
  }
  void redo() override
  {
    add();
  }
};

class MoveNodesCommand : public Command
{
public:
  MoveNodesCommand(const std::set<nodes::Node*>& nodes, const QPointF& direction);
  void undo() override;
  void redo() override;
  [[nodiscard]] int id() const override
  {
    return MOVE_NODES_COMMAND_ID;
  }
  bool mergeWith(const QUndoCommand* command) override;

private:
  const std::map<nodes::Node*, QPointF> m_old_positions;
  QPointF m_direction;
};

}  // namespace omm
