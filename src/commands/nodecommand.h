#pragma once

#include <QPointF>
#include "commands/command.h"
#include <set>
#include <memory>

namespace omm
{

class NodeModel;
class Node;
class InputPort;
class OutputPort;
class Port;

class ConnectionCommand : public Command
{
protected:
  ConnectionCommand(const QString& label, Port& out, Port& in);
  void connect();
  void disconnect();

private:
  OutputPort& m_out;
  InputPort& m_in;
};

class ConnectPortsCommand : public ConnectionCommand
{
public:
  ConnectPortsCommand(Port& a, Port& b);
  void undo() override { disconnect(); }
  void redo() override { connect(); }
};

class DisconnectPortsCommand : public ConnectionCommand
{
public:
  DisconnectPortsCommand(InputPort& port);
  void undo() override { connect(); }
  void redo() override { disconnect(); }
};


class NodeCommand : public Command
{
protected:
  NodeCommand(const QString& label, NodeModel& model,
              std::vector<Node*> refs, std::vector<std::unique_ptr<Node>> owns);
  void remove();
  void add();

private:
  std::vector<Node*> m_refs;
  std::vector<std::unique_ptr<Node>> m_owns;
  NodeModel& m_model;
  std::list<DisconnectPortsCommand> m_destroyed_connections;
};

class RemoveNodesCommand : public NodeCommand
{
public:
  RemoveNodesCommand(NodeModel& model, std::vector<Node*> nodes);
  void undo() override { add(); }
  void redo() override { remove(); }
};

class AddNodesCommand : public NodeCommand
{
public:
  AddNodesCommand(NodeModel& model, std::vector<std::unique_ptr<Node>> nodes);
  void undo() override { remove(); }
  void redo() override { add(); }
};

class MoveNodesCommand : public Command
{
public:
  MoveNodesCommand(std::set<Node*> nodes, const QPointF& direction);
  void undo() override;
  void redo() override;
  int id() const override { return MOVE_NODES_COMMAND_ID; }
  bool mergeWith(const QUndoCommand* command) override;

private:
  const std::map<Node*, QPointF> m_old_positions;
  QPointF m_direction;
};

}  // namespace omm
