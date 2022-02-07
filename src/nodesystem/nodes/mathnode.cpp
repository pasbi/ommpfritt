#include "nodesystem/nodes/mathnode.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/propertyport.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "propertytypeenum.h"
#include "scene/scene.h"

namespace
{

constexpr auto glsl_definition_template = R"(
%2 %1_0(int op, %2 a, %2 b) {
  if (op == 0) {
    return a + b;
  } else if (op == 1) {
    return a - b;
  } else if (op == 2) {
    return a * b;
  } else if (op == 3) {
    return a / b;
  } else {
    // unreachable
    return %2(0.0);
  }
})";

constexpr auto python_definition_template = R"(
@listarithm_decorator
def %1(op, a, b):
    if op == 0:
        return a + b
    elif op == 1:
        return a - b
    elif op == 2:
        return a * b
    elif op == 3:
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
    else:
        # unreachable
        return 0.0;
)";

constexpr auto supported_glsl_types = std::array{
    omm::Type::Float, omm::Type::Integer,
    omm::Type::FloatVector, omm::Type::IntegerVector, omm::Type::Color
};

auto glsl_definitions()
{
  QStringList overloads;
  overloads.reserve(supported_glsl_types.size());
  const auto template_definition = QString{glsl_definition_template}.arg(omm::nodes::MathNode::TYPE);
  for (const auto& type : supported_glsl_types) {
    using omm::nodes::NodeCompilerGLSL;
    overloads.push_back(template_definition.arg(NodeCompilerGLSL::type_name(type)));
  }
  return overloads.join("\n");
}

}  // namespace

namespace omm::nodes
{

const Node::Detail MathNode::detail{
    .definitions = {
      {BackendLanguage::Python, QString{python_definition_template}.arg(MathNode::TYPE)},
      {BackendLanguage::GLSL, glsl_definitions()}
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Math")}};

MathNode::MathNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  auto& operation_property = create_property<OptionProperty>(OPERATION_PROPERTY_KEY, 0.0)
                                 .set_options({tr("+"), tr("-"), tr("*"), tr("/")})
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

QString MathNode::type() const
{
  return TYPE;
}

Type MathNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    auto type_a = find_port<InputPort>(A_VALUE_KEY)->data_type();
    const auto type_b = find_port<InputPort>(B_VALUE_KEY)->data_type();
    switch (language()) {
    case BackendLanguage::GLSL:
      return type_a;
    case BackendLanguage::Python:
      if (is_integral(type_a) && is_integral(type_b)) {
        return Type::Integer;
      } else if (is_numeric(type_a) && is_numeric(type_b)) {
        return Type::Float;
      } else if ((type_a == Type::IntegerVector || is_integral(type_a))
                 && (type_b == Type::IntegerVector || is_integral(type_b))) {
        return Type::IntegerVector;
      } else if ((is_vector(type_a) || is_numeric(type_a))
                 && (is_vector(type_b) || is_numeric(type_b))) {
        return Type::FloatVector;
      } else if ((type_a == Type::Color || is_numeric(type_a))
                 && (type_b == Type::Color || is_numeric(type_b))) {
        return Type::Color;
      } else {
        return Type::Invalid;
      }
    default:
      Q_UNREACHABLE();
    }
  }
  return Type::Invalid;
}

Type MathNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(A_VALUE_KEY), find_port<InputPort>(B_VALUE_KEY)};
  return fst_con_ptype(ports, Type::Float);
}

bool MathNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  const auto glsl_accepts_type = [with_cast, this, type, &port]() {
    auto* const a_input = find_port<InputPort>(A_VALUE_KEY);
    auto* const b_input = find_port<InputPort>(B_VALUE_KEY);
    if (a_input == nullptr || b_input == nullptr) {
      LFATAL("Unexpected condition");
    }
    const InputPort& other_port = &port == a_input ? *b_input : *a_input;
    assert((std::set{&port, &other_port} == std::set<const InputPort*>{a_input, b_input}));
    if (other_port.is_connected()) {
      if (with_cast) {
        return NodeCompilerGLSL::can_cast(type, other_port.data_type());
      } else {
        return type == other_port.data_type();
      }
    } else {
      return model().compiler().supported_types().contains(type);
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
      return is_numeric(type) || is_vector(type) || is_color(type);
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
