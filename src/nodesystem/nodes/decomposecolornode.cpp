#include "nodesystem/nodes/decomposecolornode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
def %1(c):
    return c
)";

constexpr auto glsl_definition_template = R"(
float %1_0(vec4 c) { return c.r; }
float %1_1(vec4 c) { return c.g; }
float %1_2(vec4 c) { return c.b; }
float %1_3(vec4 c) { return c.a; }
)";

}  // namespace

namespace omm::nodes
{

const Node::Detail DecomposeColorNode::detail {
    .definitions = {
      {BackendLanguage::Python, QString{python_definition_template}.arg(DecomposeColorNode::TYPE)},
      {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(DecomposeColorNode::TYPE)},
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Color")},
};

DecomposeColorNode::DecomposeColorNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<ColorProperty>(INPUT_PROPERTY_KEY, Color(Color::Model::RGBA, {0, 0, 0}, 0))
      .set_label(tr("color"))
      .set_category(category);
  add_port<OrdinaryPort<PortType::Output>>(tr("R"));
  add_port<OrdinaryPort<PortType::Output>>(tr("G"));
  add_port<OrdinaryPort<PortType::Output>>(tr("B"));
  add_port<OrdinaryPort<PortType::Output>>(tr("A"));
}

Type DecomposeColorNode::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  return Type::Float;
}

QString DecomposeColorNode::title() const
{
  return tr("Decompose Color");
}

bool DecomposeColorNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
  Q_UNUSED(port)
  return type == Type::Color;
}

QString DecomposeColorNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
