#include "nodesystem/nodes/castnode.h"
#include "properties/optionproperty.h"
#include "nodesystem/nodecompilerglsl.h"
#include <map>


namespace omm::nodes
{

bool CastNode::is_convertible(const Type from, const Type to)
{
  static const std::set<Type> scalar {Type::Integer, Type::Option, Type::Float, Type::Bool};
  static const std::set<Type> two_dim {Type::FloatVector, Type::IntegerVector};

  return (scalar.contains(from) && scalar.contains(to)) || (two_dim.contains(from) && two_dim.contains(to));
}

CastNode::CastNode(NodeModel& model, const QString& type, const Type target_type)
    : Node(model)
    , m_type(type)
    , m_target_type(target_type)
{
  m_input_port = &add_port<OrdinaryPort<PortType::Input>>(tr(""));
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr(""));
}

Type CastNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  return m_target_type;
}

bool CastNode::accepts_input_data_type(Type type, const InputPort& port) const
{
  Q_UNUSED(port)
  return is_convertible(type, m_target_type);
}

QString CastNode::function_name(const std::size_t i) const
{
  Q_UNUSED(i)
  return QString::fromStdString(std::string{variant_type_name(m_target_type)});
}

QString CastNode::title() const
{
  return CastNode::tr("Cast to %1").arg(variant_type_name(m_target_type).data());
}

QString CastNode::type() const
{
  return m_type;
}

}  // namespace omm::nodes
