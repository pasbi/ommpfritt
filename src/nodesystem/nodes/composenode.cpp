#include "nodesystem/nodes/composenode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
def %1(a, b):
  return [a, b]
)";

constexpr auto glsl_definition_template = R"(
vec2 %1_0(float a, float b) { return vec2(a, b); }
)";

}  // namespace

namespace omm::nodes
{

const Node::Detail ComposeNode::detail {
    .definitions = {
      {BackendLanguage::Python, QString{python_definition_template}.arg(ComposeNode::TYPE)},
      {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(ComposeNode::TYPE)}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Vector")},
};

ComposeNode::ComposeNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(INPUT_X_PROPERTY_KEY, 0.0)
      .set_label(tr("x"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_Y_PROPERTY_KEY, 0.0)
      .set_label(tr("y"))
      .set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("vector"));
}

Type ComposeNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output_port) {
    const auto type_a = find_port<InputPort>(*property(INPUT_X_PROPERTY_KEY))->data_type();
    const auto type_b = find_port<InputPort>(*property(INPUT_Y_PROPERTY_KEY))->data_type();

    if (is_integral(type_a) && is_integral(type_b)) {
      return Type::IntegerVector;
    } else if (is_numeric(type_a) && is_numeric(type_b)) {
      return Type::FloatVector;
    } else {
      return Type::Invalid;
    }
  }
  return Type::Invalid;
}

QString ComposeNode::title() const
{
  return tr("Compose");
}

bool ComposeNode::accepts_input_data_type(Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(port)
  Q_UNUSED(with_cast)
  return is_numeric(type);
}

QString ComposeNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
