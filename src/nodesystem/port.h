#pragma once

#include "../common.h"
#include "propertytypeenum.h"
#include <QPoint>
#include <QString>
#include <memory>
#include <set>

namespace omm::nodes
{

class Node;

enum class PortType { Input = 0x1, Output = 0x2, Both = 0x3 };
enum class PortFlavor { Ordinary, Property };

class OutputPort;
class InputPort;

class AbstractPort
{
public:
  AbstractPort(PortFlavor flavor, PortType port_type, Node& node, std::size_t index);
  virtual ~AbstractPort();
  AbstractPort(AbstractPort&&) = delete;
  AbstractPort(const AbstractPort&) = delete;
  AbstractPort& operator=(AbstractPort&&) = delete;
  AbstractPort& operator=(const AbstractPort&) = delete;

  bool is_connected(const AbstractPort* other) const;
  [[nodiscard]] bool is_connected() const;
  const PortType port_type;
  const PortFlavor flavor;
  Node& node;
  const std::size_t index;
  [[nodiscard]] virtual QString label() const = 0;
  [[nodiscard]] virtual Type data_type() const = 0;
  [[nodiscard]] QString uuid() const;
  [[nodiscard]] std::set<AbstractPort*> connected_ports() const;
};

template<PortType port_type_> class Port : public AbstractPort
{
protected:
  explicit Port(PortFlavor flavor, Node& node, std::size_t index)
      : AbstractPort(flavor, port_type_, node, index)
  {
  }
  explicit Port(const Port& other, Node& node) : AbstractPort(other, node)
  {
  }

public:
  static constexpr PortType PORT_TYPE = port_type_;
};

class InputPort : public Port<PortType::Input>
{
protected:
  InputPort(PortFlavor flavor, Node& node, std::size_t index);

public:
  InputPort(Node& node, std::size_t index);
  virtual void connect(OutputPort* port);
  [[nodiscard]] OutputPort* connected_output() const
  {
    return m_connected_output;
  }
  bool is_connected(const AbstractPort* other) const;
  [[nodiscard]] bool is_connected() const;
  [[nodiscard]] bool accepts_data_type(Type type, bool with_cast) const;

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
  OutputPort(PortFlavor flavor, Node& node, std::size_t index);

public:
  OutputPort(Node& node, std::size_t index);

  // the Tag is to protect you! Don't call OutputPort::disconnect unless you're in
  // InputPort::connect
  void disconnect(InputPort* port, InputPort::Tag);

  // the Tag is to protect you! Don't call OutputPort::connect unless you're in InputPort::connect
  void connect(InputPort* port, InputPort::Tag);
  [[nodiscard]] std::set<InputPort*> connected_inputs() const
  {
    return m_connections;
  }

  bool is_connected(const AbstractPort* other) const;
  [[nodiscard]] bool is_connected() const;

private:
  std::set<InputPort*> m_connections;
};

template<typename PortT, typename F> requires std::is_base_of_v<AbstractPort, std::decay_t<PortT>>
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
    case PortType::Both:
      Q_UNREACHABLE();
    }
  } else {
    switch (port.port_type) {
    case PortType::Input:
      return f(static_cast<InputPort&>(port));
    case PortType::Output:
      return f(static_cast<OutputPort&>(port));
    case PortType::Both:
      Q_UNREACHABLE();
    }
  }
  Q_UNREACHABLE();
  return f(static_cast<const InputPort&>(port));
}

template<PortType> struct ConcretePortSelector;
template<> struct ConcretePortSelector<PortType::Input> {
  using T = InputPort;
};
template<> struct ConcretePortSelector<PortType::Output> {
  using T = OutputPort;
};

}  // namespace omm::nodes

template<> struct omm::EnableBitMaskOperators<omm::nodes::PortType> : std::true_type {};
