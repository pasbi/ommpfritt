#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodecompiler.h"
#include "properties/floatproperty.h"
#include "properties/splineproperty.h"
#include "nodesystem/ordinaryport.h"

namespace omm
{

using namespace omm::NodeCompilerTypes;
const Node::Detail InterpolateNode::detail {
  {
    {
      AbstractNodeCompiler::Language::Python,
          QString(R"(
def %1(x, y, balance, ramp):
  t = ramp.value(balance)
  return (1-t) * x + t * y
)").arg(InterpolateNode::TYPE)
    },
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "Interpolation"),
  }
};

InterpolateNode::InterpolateNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(LEFT_VALUE_KEY, 0)
      .set_label(QObject::tr("left")).set_category(category);
  create_property<FloatProperty>(RIGHT_VALUE_KEY, 0)
      .set_label(QObject::tr("right")).set_category(category);
  create_property<FloatProperty>(BALANCE_PROPERTY_KEY, 0)
      .set_range(0.0, 1.0).set_step(0.01)
      .set_label(QObject::tr("t")).set_category(category);
  const auto linear_spline = SplineType::Initialization::Linear;
  create_property<SplineProperty>(RAMP_PROPERTY_KEY, SplineType(linear_spline, false))
      .set_label(QObject::tr("")).set_category(category);
  m_output = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString InterpolateNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    const QString type_a = find_port<InputPort>(LEFT_VALUE_KEY)->data_type();
    const QString type_b = find_port<InputPort>(RIGHT_VALUE_KEY)->data_type();
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

QString InterpolateNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports = std::vector {
    find_port<InputPort>(LEFT_VALUE_KEY),
    find_port<InputPort>(RIGHT_VALUE_KEY)
  };
  return fst_con_ptype(ports, NodeCompilerTypes::FLOAT_TYPE);
}

bool InterpolateNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  if (&port == find_port<InputPort>(RAMP_PROPERTY_KEY)
      || &port == find_port<InputPort>(BALANCE_PROPERTY_KEY)) {
    return port.data_type() == type;
  } else {
    switch (language()) {
    case AbstractNodeCompiler::Language::Python:
      return true;
    case AbstractNodeCompiler::Language::GLSL:
      return true;
    default:
      Q_UNREACHABLE();
      return true;
    }
  }
}

}  // namespace omm
