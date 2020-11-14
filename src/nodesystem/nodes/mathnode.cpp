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
using namespace omm::NodeCompilerTypes;
std::set<QString> supported_glsl_types{FLOATVECTOR_TYPE,
                                       INTEGER_TYPE,
                                       FLOAT_TYPE,
                                       COLOR_TYPE,
                                       INTEGERVECTOR_TYPE};

const QString glsl_definition_template(R"(
%1 %2_0(int op, %1 a, %1 b) {
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
    return %1(0.0);
  }
})");

}  // namespace

namespace omm
{
const Node::Detail MathNode::detail{
    {{AbstractNodeCompiler::Language::Python,
      QString(R"(
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
)")
          .arg(MathNode::TYPE)},
     {AbstractNodeCompiler::Language::GLSL,
      ::transform<QString, QList>(supported_glsl_types,
                                  [](const QString& type) {
                                    return glsl_definition_template.arg(
                                        NodeCompilerGLSL::translate_type(type));
                                  })
          .join("\n")
          .arg(MathNode::TYPE)}},
    {
        QT_TRANSLATE_NOOP("NodeMenuPath", "Math"),
    }};

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

QString MathNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    const QString type_a = find_port<InputPort>(A_VALUE_KEY)->data_type();
    const QString type_b = find_port<InputPort>(B_VALUE_KEY)->data_type();
    switch (language()) {
    case AbstractNodeCompiler::Language::GLSL:
      return type_a;
    case AbstractNodeCompiler::Language::Python:
      if (is_integral(type_a) && is_integral(type_b)) {
        return INTEGER_TYPE;
      } else if (is_numeric(type_a) && is_numeric(type_b)) {
        return FLOAT_TYPE;
      } else if ((type_a == INTEGERVECTOR_TYPE || is_integral(type_a))
                 && (type_b == INTEGERVECTOR_TYPE || is_integral(type_b))) {
        return INTEGERVECTOR_TYPE;
      } else if ((is_vector(type_a) || is_numeric(type_a))
                 && (is_vector(type_b) || is_numeric(type_b))) {
        return FLOATVECTOR_TYPE;
      } else if ((type_a == COLOR_TYPE || is_numeric(type_a))
                 && (type_b == COLOR_TYPE || is_numeric(type_b))) {
        return COLOR_TYPE;
      } else {
        return INVALID_TYPE;
      }
    default:
      Q_UNREACHABLE();
    }
  }
  return INVALID_TYPE;
}

QString MathNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(A_VALUE_KEY), find_port<InputPort>(B_VALUE_KEY)};
  return fst_con_ptype(ports, NodeCompilerTypes::FLOAT_TYPE);
}

bool MathNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  const auto glsl_accepts_type = [this, type, &port]() {
    const auto a_input = find_port<InputPort>(A_VALUE_KEY);
    const auto b_input = find_port<InputPort>(B_VALUE_KEY);
    const InputPort& other_port = &port == a_input ? *b_input : *a_input;
    assert((std::set{&port, &other_port} == std::set<const InputPort*>{a_input, b_input}));
    if (other_port.is_connected()) {
      return type == other_port.data_type();
    } else {
      return ::contains(supported_glsl_types, type);
    }
  };

  assert(&port.node == this);
  if (&port == m_operation_input) {
    return port.data_type() == type;
  } else {
    switch (language()) {
    case AbstractNodeCompiler::Language::GLSL:
      return glsl_accepts_type();
    case AbstractNodeCompiler::Language::Python:
      return is_numeric(type) || is_vector(type) || type == COLOR_TYPE;
    default:
      Q_UNREACHABLE();
      return true;
    }
  }
}

QString MathNode::title() const
{
  auto&& opp = static_cast<const OptionProperty&>(*property(OPERATION_PROPERTY_KEY));
  const std::size_t i = opp.value();
  QString operation_label = tr("invalid");
  try {
    operation_label = opp.options().at(i);
  } catch (const std::out_of_range&) {
  }

  return Node::title() + tr(" [%1]").arg(operation_label);
}

}  // namespace omm
