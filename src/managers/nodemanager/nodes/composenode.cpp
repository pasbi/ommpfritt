#include "managers/nodemanager/nodes/composenode.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

const Node::Detail ComposeNode::detail { {
    { AbstractNodeCompiler::Language::Python, QString(R"(
def %1(a, b):
  return [a, b]
)").arg(ComposeNode::TYPE) },
    { AbstractNodeCompiler::Language::GLSL, QString(R"(
vec2 %1(float a, float b) { return vec2(a, b); }
)").arg(ComposeNode::TYPE) }
    } };

ComposeNode::ComposeNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(INPUT_X_PROPERTY_KEY, 0.0)
      .set_label(tr("x")).set_category(category);
  create_property<FloatProperty>(INPUT_Y_PROPERTY_KEY, 0.0)
      .set_label(tr("y")).set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("vector"));
}

QString ComposeNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_output_port) {
    const QString type_a = find_port<InputPort>(*property(INPUT_X_PROPERTY_KEY))->data_type();
    const QString type_b = find_port<InputPort>(*property(INPUT_Y_PROPERTY_KEY))->data_type();

    if (is_integral(type_a) && is_integral(type_b)) {
      return INTEGERVECTOR_TYPE;
    } else if (is_numeric(type_a) && is_numeric(type_b)) {
      return FLOATVECTOR_TYPE;
    } else {
      return INVALID_TYPE;
    }
  }
  return INVALID_TYPE;
}

QString ComposeNode::title() const
{
  return tr("Compose");
}

bool ComposeNode::accepts_input_data_type(const QString &type, const InputPort &port) const
{
  Q_UNUSED(port)
  return NodeCompilerTypes::is_numeric(type);
}

}  // namespace omm
