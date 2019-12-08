#pragma once

#include <set>
#include <QString>
#include <QPoint>

namespace omm
{

class Node;

class Port
{
public:
  explicit Port(bool is_input, Node& node, std::size_t index, const QString& name);
  virtual ~Port();
  virtual bool is_connected(const Port* other) const = 0;
  const QString name;
  Node& node;
  const std::size_t index;
  const bool is_input;
};

class OutputPort;

class InputPort : public Port
{
public:
  InputPort(Node& node, std::size_t index, const QString& name);
  static constexpr bool IS_INPUT = true;
  bool is_connected(const Port* other) const override;
  void connect(OutputPort* port);
  OutputPort* connection() const { return m_connection; }

  class Tag
  {
  private:
    Tag() = default;
    friend class InputPort;
  };

private:
  OutputPort* m_connection = nullptr;
};

class OutputPort : public Port
{
public:
  OutputPort(Node& node, std::size_t index, const QString& name);
  static constexpr bool IS_INPUT = false;
  bool is_connected(const Port* other) const override;

  // the Tag is to protect you! Don't call OutputPort::disconnect unless you're in InputPort::connect
  void disconnect(InputPort* port, InputPort::Tag);

  // the Tag is to protect you! Don't call OutputPort::connect unless you're in InputPort::connect
  void connect(InputPort* port, InputPort::Tag);

private:
  std::set<InputPort*> m_connections;
};

}
