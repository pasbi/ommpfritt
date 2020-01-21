#include "managers/nodemanager/nodes/mathnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace
{

using namespace omm::NodeCompilerTypes;
enum class Operation { Addition, Difference, Multiplication, Division, Power };

}  // namespace

namespace omm
{

const Node::Detail MathNode::detail { { AbstractNodeCompiler::Language::Python } };

MathNode::MathNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  create_property<OptionsProperty>(OPERATION_PROPERTY_KEY, 0.0)
      .set_options({ tr("+"), tr("-"), tr("*"), tr("/"), tr("pow") })
      .set_label(QObject::tr("Operation")).set_category(category);
  create_property<FloatProperty>(A_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("a")).set_category(category);
  create_property<FloatProperty>(B_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("b")).set_category(category);
  m_result_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString MathNode::definition() const
{
  switch (language()) {
  case AbstractNodeCompiler::Language::Python:
    return QString(R"(
def %1(op, a, b):
    import numpy as np
    def do_op(op, a, b):
        if op == 0:
            return a + b
        elif op == 1:
            return a - b
        elif op == 2:
            return a * b
        elif op == 3:
            return a / b
        elif op == 4:
            return a ** b
        else:
            return 0.0;
    if isinstance(a, list):
        a = np.array(a)
    if isinstance(b, list):
        b = np.array(b)
    result = do_op(op, a, b)
    if isinstance(result, np.ndarray):
        result = list(result)
    return result
)").arg(uuid());
  case AbstractNodeCompiler::Language::GLSL:
    return "";
  default:
    Q_UNREACHABLE();
    return "";
  }
}

QString MathNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_result_port) {
    const QString type_a = find_port<InputPort>(*property(A_PROPERTY_KEY))->data_type();
    const QString type_b = find_port<InputPort>(*property(B_PROPERTY_KEY))->data_type();
    switch (language()) {
    case AbstractNodeCompiler::Language::GLSL:
      if (type_a == type_b) {
        return type_a;
      } else {
        return INVALID_TYPE;
      }
    case AbstractNodeCompiler::Language::Python:
      using namespace NodeCompilerTypes;
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

bool MathNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  assert(&port.node == this);
  assert(port.flavor == PortFlavor::Property);
  const auto* property = static_cast<const PropertyPort<PortType::Input>&>(port).property();
  if (property == this->property(OPERATION_PROPERTY_KEY)) {
    return Node::accepts_input_data_type(type, port);
  } else {
    // Math Node input shall accept anything
    return true;
  }
}

QString MathNode::title() const
{
  auto&& opp = static_cast<const OptionsProperty&>(*property(OPERATION_PROPERTY_KEY));
  const std::size_t i = opp.value();
  QString operation_label = tr("invalid");
  try {
    operation_label = opp.options().at(i);
  } catch (const std::out_of_range&) {
  }

  return Node::title() + tr(" [%1]").arg(operation_label);
}

}  // namespace
