#include "managers/nodemanager/nodes/composecolornode.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

const Node::Detail ComposeColorNode::detail { {
    { AbstractNodeCompiler::Language::Python, QString(R"(
def %1(r, g, b, a):
  return [r, g, b, a]
)").arg(ComposeColorNode::TYPE) },
    { AbstractNodeCompiler::Language::GLSL, QString(R"(
vec4 %1_0(float r, float g, float b, float a) { return vec4(r, g, b, a); }
)").arg(ComposeColorNode::TYPE) }
    } };

ComposeColorNode::ComposeColorNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(INPUT_R_PROPERTY_KEY, 0.0)
      .set_label(tr("r")).set_category(category);
  create_property<FloatProperty>(INPUT_G_PROPERTY_KEY, 0.0)
      .set_label(tr("g")).set_category(category);
  create_property<FloatProperty>(INPUT_B_PROPERTY_KEY, 0.0)
      .set_label(tr("b")).set_category(category);
  create_property<FloatProperty>(INPUT_A_PROPERTY_KEY, 0.0)
      .set_label(tr("a")).set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("color"));
}

QString ComposeColorNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  using namespace NodeCompilerTypes;
  return COLOR_TYPE;
}

QString ComposeColorNode::title() const
{
  return tr("Compose Color");
}

bool ComposeColorNode::accepts_input_data_type(const QString &type, const InputPort &port) const
{
  Q_UNUSED(port)
  return NodeCompilerTypes::is_numeric(type);
}

}  // namespace omm
