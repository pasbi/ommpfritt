#pragma once

#include "nodesystem/node.h"
#include "nodesystem/ordinaryport.h"

namespace omm::nodes
{

class CastNodeBase : public Node
{
  Q_OBJECT
public:
  explicit CastNodeBase(NodeModel& model) : Node(model)
  {
    m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr(""));
    m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr(""));
  }
  OutputPort* m_output_port;
  InputPort* m_input_port;

protected:
  static bool is_convertible(const QString& from, const QString& to);
  static QString translate_type(const QString& type);
};

template<const char* target_type>
class CastNode : public CastNodeBase
{
public:
  using CastNodeBase::CastNodeBase;
  [[nodiscard]] QString output_data_type(const OutputPort& port) const override
  {
    Q_UNUSED(port);
    return target_type;
  }

  [[nodiscard]] bool accepts_input_data_type(const QString& type, const InputPort& port) const override
  {
    Q_UNUSED(port)
    return is_convertible(type, target_type);
  }

  [[nodiscard]] QString function_name(const std::size_t i) const override
  {
    Q_UNUSED(i)
    return translate_type(target_type);
  }

  static const Detail detail;
  [[nodiscard]] QString title() const override
  {
    return CastNodeBase::tr("Cast to %1").arg(target_type);
  }
};

}  // namespace omm::nodes
