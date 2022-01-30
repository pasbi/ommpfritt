#include "nodesystem/nodes/castnode.h"
#include "properties/optionproperty.h"
#include "nodesystem/nodecompilerglsl.h"
#include <map>


namespace omm::nodes
{

bool CastNode::is_convertible(const QString& from, const QString& to)
{
  static const std::set<QString> scalar {types::INTEGER_TYPE, types::OPTION_TYPE, types::FLOAT_TYPE, types::BOOL_TYPE};
  static const std::set<QString> two_dim {types::FLOATVECTOR_TYPE, types::INTEGERVECTOR_TYPE};

  return (scalar.contains(from) && scalar.contains(to)) || (two_dim.contains(from) && two_dim.contains(to));
}

CastNode::CastNode(NodeModel& model, const QString& type, const QString& target_type)
    : Node(model)
    , m_type(type)
    , m_target_type(target_type)
{
  m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr(""));
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr(""));
}

QString CastNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port);
  return m_target_type;
}

bool CastNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return is_convertible(type, m_target_type);
}

QString CastNode::function_name(const std::size_t i) const
{
  Q_UNUSED(i)
  return NodeCompilerGLSL::translate_type(m_target_type);
}

QString CastNode::title() const
{
  return CastNode::tr("Cast to %1").arg(m_target_type);
}

QString CastNode::type() const
{
  return m_type;
}

}  // namespace omm::nodes
