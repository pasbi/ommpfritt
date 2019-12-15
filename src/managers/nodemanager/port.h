#pragma once

#include <set>
#include <QString>
#include <QPoint>

namespace omm
{

class Node;

enum class PortType { Input, Output };
enum class PortFlavor { Ordinary, Property };

class OutputPort;
class InputPort;

class AbstractPort
{
public:
  AbstractPort(PortFlavor flavor, PortType port_type, Node& node, std::size_t index, const QString& name);
  virtual ~AbstractPort();
  bool is_connected(const AbstractPort* other) const;
  bool is_connected() const;
  const PortType port_type;
  const PortFlavor flavor;
  Node& node;
  const std::size_t index;
  const QString name;
};

template<typename PortT,
         typename F,
         class = std::enable_if<std::is_base_of_v<AbstractPort, std::decay_t<PortT>>>>
decltype(auto) visit(PortT&& port, F&& f)
{
  // hand over port by (const) reference, but not as pointer.
  static_assert(!std::is_pointer_v<PortT>);
  if constexpr (std::is_const_v<std::remove_reference_t<PortT>>) {
    switch (port.port_type) {
    case PortType::Input:
      return f(static_cast<const InputPort&>(port));
    case PortType::Output:
      return f(static_cast<const OutputPort&>(port));
    }
  } else {
    switch (port.port_type) {
    case PortType::Input:
      return f(static_cast<InputPort&>(port));
    case PortType::Output:
      return f(static_cast<OutputPort&>(port));
    }
  }
  Q_UNREACHABLE();
  return f(static_cast<const InputPort&>(port));
}

template<PortType port_type_> class Port : public AbstractPort
{
protected:
  explicit Port(PortFlavor flavor, Node& node, std::size_t index, const QString& name)
    : AbstractPort(flavor, port_type_, node, index, name) {}
public:
  static constexpr PortType PORT_TYPE = port_type_;
};

class InputPort : public Port<PortType::Input>
{
protected:
  InputPort(PortFlavor flavor, Node& node, std::size_t index, const QString& name);
public:
  InputPort(Node& node, std::size_t index, const QString& name);
  void connect(OutputPort* port);
  OutputPort* connected_output() const { return m_connected_output; }
  bool is_connected(const AbstractPort* other) const;
  bool is_connected() const;

  class Tag
  {
  private:
    Tag() = default;
    friend class InputPort;
  };

private:
  OutputPort* m_connected_output = nullptr;
};

class OutputPort : public Port<PortType::Output>
{
protected:
  OutputPort(PortFlavor flavor, Node& node, std::size_t index, const QString& name);
public:
  OutputPort(Node& node, std::size_t index, const QString& name);

  // the Tag is to protect you! Don't call OutputPort::disconnect unless you're in InputPort::connect
  void disconnect(InputPort* port, InputPort::Tag);

  // the Tag is to protect you! Don't call OutputPort::connect unless you're in InputPort::connect
  void connect(InputPort* port, InputPort::Tag);
  std::set<InputPort*> connected_inputs() const { return m_connections; }

  bool is_connected(const AbstractPort* other) const;
  bool is_connected() const;

private:
  std::set<InputPort*> m_connections;
};

template<PortType> struct ConcretePortSelector;
template<> struct ConcretePortSelector<PortType::Input> { using T = InputPort; };
template<> struct ConcretePortSelector<PortType::Output> { using T = OutputPort; };

}
