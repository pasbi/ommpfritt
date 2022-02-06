#include "nodesystem/nodes/composecolornode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
def %1(r, g, b, a):
  return [r, g, b, a]
)";

constexpr auto glsl_definition_template = R"(
vec4 %1_0(float r, float g, float b, float a) { return vec4(r, g, b, a); }
)";

}  // namespace

namespace omm::nodes
{

const Node::Detail ComposeColorNode::detail {
    .definitions = {
       {BackendLanguage::Python, QString{python_definition_template}.arg(ComposeColorNode::TYPE)},
       {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(ComposeColorNode::TYPE)}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Color")},
};

ComposeColorNode::ComposeColorNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  static constexpr double STEP = 0.01;
  create_property<FloatProperty>(INPUT_R_PROPERTY_KEY, PortType::Input, 0.0)
      .set_step(STEP)
      .set_range(0.0, 1.0)
      .set_label(tr("R"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_G_PROPERTY_KEY, PortType::Input, 0.0)
      .set_step(STEP)
      .set_range(0.0, 1.0)
      .set_label(tr("G"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_B_PROPERTY_KEY, PortType::Input, 0.0)
      .set_step(STEP)
      .set_range(0.0, 1.0)
      .set_label(tr("B"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_A_PROPERTY_KEY, PortType::Input, 0.0)
      .set_step(STEP)
      .set_range(0.0, 1.0)
      .set_label(tr("A"))
      .set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("color"));
}

Type ComposeColorNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  return Type::Color;
}

QString ComposeColorNode::title() const
{
  return tr("Compose Color");
}

bool ComposeColorNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
  Q_UNUSED(port)
  return is_numeric(type);
}

QString ComposeColorNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
