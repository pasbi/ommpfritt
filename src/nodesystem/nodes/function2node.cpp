#include "nodesystem/nodes/function2node.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
import math
def %1(op, x, y):
  if op == 0:
    return math.atan2(y, x)
  elif op == 1:
    return math.sqrt(x**2.0 + y**2.0)
  elif op == 2:
    return math.pow(x, y)
  elif op == 3:
    return math.min(x, y)
  elif (op == 4:
    return math.max(x, y)
  else:
    return 0.0
)";

constexpr auto glsl_definition_template = R"(
float %1_0(int op, float x, float y) {
  if (op == 0) {
    return atan(y, x);
  } else if (op == 1) {
    return length(vec2(x, y));
  } else if (op == 2) {
    return pow(x, y);
  } else if (op == 3) {
    return min(x, y);
  } else if (op == 4) {
    return max(x, y);
  } else {
    return 0.0;
  }
}
float %1_0(int op, int x, int y) {
  return %1_0(op, float(x), float(y));
}
)";

}  // namespace

namespace omm::nodes
{

const Node::Detail Function2Node::detail{
    .definitions = {
       {BackendLanguage::Python, QString{python_definition_template}.arg(Function2Node::TYPE)},
       {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(Function2Node::TYPE)}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Math")},
};

Function2Node::Function2Node(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<OptionProperty>(OPERATION_PROPERTY_KEY, 0)
      .set_options({tr("atan2"), tr("length"), tr("pow"), tr("min"), tr("max")})
      .set_label(QObject::tr("Operation"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_A_PROPERTY_KEY, 0.0)
      .set_label(tr("a"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_B_PROPERTY_KEY, 0.0)
      .set_label(tr("b"))
      .set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

Type Function2Node::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  return Type::FloatVector;
}

bool Function2Node::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
  Q_UNUSED(port)
  return is_numeric(type);
}

QString Function2Node::type() const
{
  return TYPE;
}

QString Function2Node::title() const
{
  auto&& opp = dynamic_cast<const OptionProperty&>(*property(OPERATION_PROPERTY_KEY));
  const std::size_t i = opp.value();
  QString operation_label = tr("invalid");
  try {
    operation_label = opp.options().at(i);
  } catch (const std::out_of_range&) {
  }

  return Node::title() + tr(" [%1]").arg(operation_label);
}

}  // namespace omm::nodes
