#include "nodesystem/nodes/mathnode.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "nodesystem/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionproperty.h"

namespace
{

using namespace omm::NodeCompilerTypes;
std::set<QString> supported_glsl_types {
  FLOATVECTOR_TYPE, INTEGER_TYPE, FLOAT_TYPE, COLOR_TYPE, INTEGERVECTOR_TYPE
};

}  // namespace

namespace omm
{

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

const Node::Detail MathNode::detail {
  {
    {
      AbstractNodeCompiler::Language::Python,
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
        return a / b
    else:
        # unreachable
        return 0.0;
)").arg(MathNode::TYPE)
    },
    {
      AbstractNodeCompiler::Language::GLSL,
      ::transform<QString, QList>(supported_glsl_types, [](const QString& type) {
        return glsl_definition_template.arg(NodeCompilerGLSL::translate_type(type));
      }).join("\n").arg(MathNode::TYPE)
    }
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "Math"),
  }
};

MathNode::MathNode(NodeModel& model)
  : Node(model)
{
  const QString category = tr("Node");
  auto& operation_property = create_property<OptionProperty>(OPERATION_PROPERTY_KEY, 0.0)
      .set_options({ tr("+"), tr("-"), tr("*"), tr("/") })
      .set_label(QObject::tr("Operation")).set_category(category);
  m_a_input = &add_port<OrdinaryPort<PortType::Input>>(tr("a"));
  m_b_input = &add_port<OrdinaryPort<PortType::Input>>(tr("b"));
  m_output = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
  m_operation_input = find_port<InputPort>(operation_property);
}

QString MathNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    const QString type_a = m_a_input->data_type();
    const QString type_b = m_b_input->data_type();
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
  return fst_con_ptype({ m_a_input, m_b_input }, NodeCompilerTypes::FLOAT_TYPE);
}

bool MathNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  const auto glsl_accepts_type = [this, type, &port]() {
    const InputPort& other_port = &port == m_a_input ? *m_b_input : *m_a_input;
    using InputSet = std::set<const InputPort*>;
    assert((InputSet { &port, &other_port }) == (InputSet { m_a_input, m_b_input }));
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
      // Math Node input shall accept anything
      return true;
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

}  // namespace
