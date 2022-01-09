#include "nodesystem/nodes/function2node.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"

namespace
{

constexpr auto GLSL = omm::nodes::BackendLanguage::GLSL;
constexpr auto Python = omm::nodes::BackendLanguage::Python;

constexpr auto operations = std::array {QT_TR_NOOP("atan2"), QT_TR_NOOP("length"),
                                        QT_TR_NOOP("pow"), QT_TR_NOOP("min"), QT_TR_NOOP("max")};

template<omm::nodes::BackendLanguage>
QString generate_condition(const int i, const QString&)
{
  return QString{"op == %1"}.arg(i);
}

template<omm::nodes::BackendLanguage language>
QString generate_return_value(const QString& function_name)
{
  if constexpr (language == Python) {
    if (function_name == "atan2") {
      return "math.atan2(y, x)";
    } else if (function_name == "length") {
      return "math.sqrt(x**2.0 + y **2.0)";
    } else {
      return "math." + function_name + "(x, y)";
    }
  } else {
    if (function_name == "atan") {
      return "atan(y, x)";
    } else if (function_name == "length") {
      return "length(vec2(x, y))";
    } else {
      return function_name + "(x, y)";
    }
  }
}

constexpr auto glsl_definition_template = R"(
%3 %1_0(int op, %3 x, %3 y) {
%2
}
%3 %1_1(int op, %3 v) {
  return %1_0(op, v);
}
)";

constexpr auto python_definition_template = R"(
import math
def %1(op, x, y):
%2
)";

template<omm::nodes::BackendLanguage language> QString definition()
{
  using omm::nodes::codegeneration::if_else_chain;
  using omm::nodes::codegeneration::indent;
  const auto chain = if_else_chain<language>(operations,
                                             generate_condition<language>,
                                             generate_return_value<language>);
  return QString{language == GLSL ? glsl_definition_template : python_definition_template}
            .arg(omm::nodes::Function2Node::TYPE, indent(chain, 1));
}

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

QString Function2Node::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  return types::FLOAT_TYPE;
}

bool Function2Node::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return types::is_numeric(type);
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
