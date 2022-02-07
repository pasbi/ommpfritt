#include "nodesystem/nodes/decomposenode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
def %1(v):
  return v
)";

constexpr auto glsl_definition_template = R"(
float %1_0(vec2 xy) { return xy.x; }
float %1_1(vec2 xy) { return xy.y; }
)";

}  // namespace

namespace omm::nodes
{

const Node::Detail DecomposeNode::detail {
    .definitions = {
        {BackendLanguage::Python, QString{python_definition_template}.arg(DecomposeNode::TYPE)},
        {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(DecomposeNode::TYPE)}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Vector")},
};

DecomposeNode::DecomposeNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatVectorProperty>(INPUT_PROPERTY_KEY, Vec2f(0.0, 0.0))
      .set_label(tr("vector"))
      .set_category(category);
  m_output_x_port = &add_port<OrdinaryPort<PortType::Output>>(tr("x"));
  m_output_y_port = &add_port<OrdinaryPort<PortType::Output>>(tr("y"));
}

Type DecomposeNode::output_data_type(const OutputPort& port) const
{
  switch (language()) {
  case BackendLanguage::Python:
    if (&port == m_output_x_port || &port == m_output_y_port) {
      const auto type = find_port<InputPort>(*property(INPUT_PROPERTY_KEY))->data_type();
      if (type == Type::IntegerVector) {
        return Type::Integer;
      } else if (type == Type::FloatVector) {
        return Type::Float;
      }
    }
    return Type::Invalid;
  case BackendLanguage::GLSL:
    return Type::Float;
  }
  Q_UNREACHABLE();
  return Type::Invalid;
}

QString DecomposeNode::title() const
{
  return tr("Decompose");
}

bool DecomposeNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
  Q_UNUSED(port)
  switch (language()) {
  case BackendLanguage::Python:
    return is_vector(type);
  case BackendLanguage::GLSL:
    return type == Type::FloatVector;
  default:
    Q_UNREACHABLE();
    return false;
  }
}

QString DecomposeNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
