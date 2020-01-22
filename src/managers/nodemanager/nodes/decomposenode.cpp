#include "managers/nodemanager/nodes/decomposenode.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

const Node::Detail DecomposeNode::detail { { AbstractNodeCompiler::Language::Python } };

DecomposeNode::DecomposeNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatVectorProperty>(INPUT_PROPERTY_KEY, Vec2f(0.0, 0.0))
      .set_label(tr("in")).set_category(category);
  m_output_x_port = &add_port<OrdinaryPort<PortType::Output>>(tr("x"));
  m_output_y_port = &add_port<OrdinaryPort<PortType::Output>>(tr("y"));
}

QString DecomposeNode::definition() const
{
  return QString(R"(
def %1(v):
  return v
)").arg(uuid());
}

QString DecomposeNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_output_x_port || &port == m_output_y_port) {
    const QString type = find_port<InputPort>(*property(INPUT_PROPERTY_KEY))->data_type();
    if (type == INTEGERVECTOR_TYPE) {
      return INTEGER_TYPE;
    } else if (type == FLOATVECTOR_TYPE) {
      return FLOAT_TYPE;
    }
  }
  return INVALID_TYPE;
}

QString DecomposeNode::title() const
{
  return tr("Decompose");
}

bool DecomposeNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  using namespace NodeCompilerTypes;
  Q_UNUSED(port)
  return is_vector(type);
}

}  // namespace omm
