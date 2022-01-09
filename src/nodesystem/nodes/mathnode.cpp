#include "nodesystem/nodes/mathnode.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/propertyport.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include "variant.h"

namespace
{

constexpr auto GLSL = omm::nodes::BackendLanguage::GLSL;
constexpr auto Python = omm::nodes::BackendLanguage::Python;

namespace types = omm::nodes::types;

static constexpr auto operations = std::array{QT_TR_NOOP("+"), QT_TR_NOOP("-"), QT_TR_NOOP("*"),
                                              QT_TR_NOOP("/")};

constexpr auto glsl_definition_template = R"(
%3 %1_0(int op, %3 a, %3 b) {
%2
}
%3 %1_1(int op, %3 a, %3 b) {
  return %1_0(op, v);
}
)";

constexpr auto python_definition_template = R"(
import math
@listarithm_decorator
def %1(op, v):
%2
)";

constexpr auto python_divide = R"(
import numpy as np
def isinteger(v):
    if isinstance(v, int):
        return True
    elif isinstance(v, np.ndarray):
        return issubclass(v.dtype.type, np.integer)
    else:
        return False

if isinstance(a, int) and isinstance(b, int):
  return int(a / b)
elif  isinteger(a) and isinteger(b):
    return (a / b).astype(np.int)
else:
  return a / b
)";

template<omm::nodes::BackendLanguage>
QString generate_condition(const int i, const QString&)
{
  return QString{"op == %1"}.arg(i);
}

template<omm::nodes::BackendLanguage language>
QString generate_return_value(const QString& function_name)
{
  if (language == Python && function_name == "/") {
    return python_divide;
  } else {
    return QString("a %1 b").arg(function_name);
  }
}

template<omm::nodes::BackendLanguage language> QString definition()
{
  using omm::nodes::codegeneration::if_else_chain;
  using omm::nodes::codegeneration::indent;
  const auto chain = if_else_chain<language>(operations,
                                             generate_condition<language>,
                                             generate_return_value<language>);
  return QString{language == GLSL ? glsl_definition_template : python_definition_template}
      .arg(omm::nodes::MathNode::TYPE, indent(chain, 1));
}

struct Overload
{
  std::string_view type;
  auto types() const
  {
    return std::array {type};
  }
  bool operator==(std::string_view v) const
  {
    return v == type;
  }
};

constexpr std::array overloads {
      Overload{.type = types::FLOATVECTOR_TYPE},
      Overload{.type = types::INTEGER_TYPE},
      Overload{.type = types::FLOAT_TYPE},
      Overload{.type = types::COLOR_TYPE},
      Overload{.type = types::INTEGERVECTOR_TYPE}
};

}  // namespace

namespace omm::nodes
{

const Node::Detail MathNode::detail{
    .definitions = {
      {BackendLanguage::Python, definition<Python>()},
      {BackendLanguage::GLSL, codegeneration::overload(definition<GLSL>(), overloads)}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Math")}};

MathNode::MathNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  auto& operation_property = create_property<OptionProperty>(OPERATION_PROPERTY_KEY, 0.0)
      .set_options(::transform<QString, std::vector>(operations, [](const auto& op) { return tr(op); }))
      .set_label(QObject::tr("Operation"))
      .set_category(category);
  create_property<FloatProperty>(A_VALUE_KEY, PortType::Input, 0)
      .set_label(QObject::tr("a"))
      .set_category(category);
  create_property<FloatProperty>(B_VALUE_KEY, PortType::Input, 0)
      .set_label(QObject::tr("b"))
      .set_category(category);
  m_output = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
  m_operation_input = find_port<InputPort>(operation_property);
}

QString MathNode::output_data_type(const OutputPort& port) const
{
  using types::is_vector;
  if (&port == m_output) {
    QString type_a = find_port<InputPort>(A_VALUE_KEY)->data_type();
    const QString type_b = find_port<InputPort>(B_VALUE_KEY)->data_type();
    switch (language()) {
    case BackendLanguage::GLSL:
      return type_a;
    case BackendLanguage::Python:
      if (types::is_integral(type_a) && types::is_integral(type_b)) {
        return types::INTEGER_TYPE;
      } else if (types::is_numeric(type_a) && types::is_numeric(type_b)) {
        return types::FLOAT_TYPE;
      } else if ((type_a == types::INTEGERVECTOR_TYPE || types::is_integral(type_a))
                 && (type_b == types::INTEGERVECTOR_TYPE || types::is_integral(type_b))) {
        return types::INTEGERVECTOR_TYPE;
      } else if ((is_vector(type_a) || types::is_numeric(type_a))
                 && (is_vector(type_b) || types::is_numeric(type_b))) {
        return types::FLOATVECTOR_TYPE;
      } else if ((type_a == types::COLOR_TYPE || types::is_numeric(type_a))
                 && (type_b == types::COLOR_TYPE || types::is_numeric(type_b))) {
        return types::COLOR_TYPE;
      } else {
        return types::INVALID_TYPE;
      }
    default:
      Q_UNREACHABLE();
    }
  }
  return types::INVALID_TYPE;
}

QString MathNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(A_VALUE_KEY), find_port<InputPort>(B_VALUE_KEY)};
  return fst_con_ptype(ports, types::FLOAT_TYPE);
}

bool MathNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  const auto glsl_accepts_type = [this, type, &port]() {
    auto* const a_input = find_port<InputPort>(A_VALUE_KEY);
    auto* const b_input = find_port<InputPort>(B_VALUE_KEY);
    if (a_input == nullptr || b_input == nullptr) {
      LFATAL("Unexpected condition");
    }
    const InputPort& other_port = &port == a_input ? *b_input : *a_input;
    assert((std::set{&port, &other_port} == std::set<const InputPort*>{a_input, b_input}));
    if (other_port.is_connected()) {
      return type == other_port.data_type();
    } else {
      return ::contains(overloads, std::string_view{type.toStdString().c_str()});
    }
  };

  assert(&port.node == this);
  if (&port == m_operation_input) {
    return port.data_type() == type;
  } else {
    switch (language()) {
    case BackendLanguage::GLSL:
      return glsl_accepts_type();
    case BackendLanguage::Python:
      return types::is_numeric(type) || types::is_vector(type) || type == types::COLOR_TYPE;
    default:
      Q_UNREACHABLE();
      return true;
    }
  }
}

QString MathNode::title() const
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

}  // namespace omm
