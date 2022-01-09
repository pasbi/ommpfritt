#include "nodesystem/nodes/functionnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/common.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"

namespace
{

constexpr auto GLSL = omm::nodes::BackendLanguage::GLSL;
constexpr auto Python = omm::nodes::BackendLanguage::Python;

constexpr auto operations = std::array {QT_TR_NOOP("abs"), QT_TR_NOOP("sqrt"), QT_TR_NOOP("log"),
                                        QT_TR_NOOP("log2"), QT_TR_NOOP("exp"), QT_TR_NOOP("exp2"),
                                        QT_TR_NOOP("sin"), QT_TR_NOOP("cos"), QT_TR_NOOP("tan"),
                                        QT_TR_NOOP("asin"), QT_TR_NOOP("acos"), QT_TR_NOOP("atan"),
                                        QT_TR_NOOP("fract"), QT_TR_NOOP("ceil"),
                                        QT_TR_NOOP("floor"), QT_TR_NOOP("sign"),
                                        QT_TR_NOOP("radians"), QT_TR_NOOP("degrees")};

template<omm::nodes::BackendLanguage>
QString generate_condition(const int i, const QString&)
{
  return QString{"op == %1"}.arg(i);
}

template<omm::nodes::BackendLanguage language>
QString generate_return_value(const QString& function_name)
{
  if constexpr (language == GLSL) {
    return function_name + "(v)";
  } else {
    static constexpr auto define_math_function = [](const auto& name) {
      return QString{"math.%1(v)"}.arg(name);
    };
    if (function_name == "abs") {
      return define_math_function("fabs");
    } else if (function_name == "exp2") {
      return QString{"2 ** v"};
    } else if (function_name == "fract") {
      return QString{"v - math.floor(v)"};
    } else {
      return define_math_function(function_name);
    }
  }
}

constexpr auto glsl_definition_template = R"(
float %1_0(int op, float v) {
%2
}
float %1_1(int op, float v) {
  return %1_0(op, v);
}
)";

constexpr auto python_definition_template = R"(
import math
def %1(op, v):
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
            .arg(omm::nodes::FunctionNode::TYPE, indent(chain, 1));
}

}  // namespace omm

namespace omm::nodes
{

const Node::Detail FunctionNode::detail{
    .definitions = {
      {BackendLanguage::Python, definition<Python>()},
      {BackendLanguage::GLSL, definition<GLSL>()}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Math")},
};

FunctionNode::FunctionNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<OptionProperty>(OPERATION_PROPERTY_KEY, 0)
      .set_options(::transform<QString, std::vector>(operations, [](const auto& op) { return tr(op); }))
      .set_label(QObject::tr("Operation"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_A_PROPERTY_KEY, 0.0)
      .set_label(tr("x"))
      .set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString FunctionNode::output_data_type(const OutputPort&) const
{
  return types::FLOAT_TYPE;
}

bool FunctionNode::accepts_input_data_type(const QString& type, const InputPort&) const
{
  return types::is_numeric(type);
}

QString FunctionNode::title() const
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
