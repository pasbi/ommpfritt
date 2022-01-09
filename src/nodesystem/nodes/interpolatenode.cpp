#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/splineproperty.h"

namespace
{

namespace types = omm::nodes::types;
const std::set<std::array<QString, 2>> supported_glsl_types {
  {types::FLOATVECTOR_TYPE, types::FLOATVECTOR_TYPE },
  {types::FLOAT_TYPE, types::INTEGER_TYPE},
  {types::FLOAT_TYPE, types::FLOAT_TYPE},
  {types::COLOR_TYPE, types::COLOR_TYPE},
  {types::FLOATVECTOR_TYPE, types::INTEGERVECTOR_TYPE}
};

const QString glsl_definition_template
    = QString(R"(
%2 %4_0(%3 a, %3 b, float t, %1 spline) {
  const int n = SPLINE_SIZE - 1;
  int k = int(t * n);
  int i = clamp(k, 0, n);
  int j = clamp(i+1, 0, n);
  float r = t * n - k;
  float s = mix(spline[i], spline[j], r);
  return mix(a, b, s);
}
)")
          .arg(omm::nodes::NodeCompilerGLSL::translate_type(types::SPLINE_TYPE));

}  // namespace

namespace omm::nodes
{

const Node::Detail InterpolateNode::detail{
    .definitions = {
        {BackendLanguage::Python,
         QString(R"(
def %1(x, y, balance, ramp):
  t = ramp.value(balance)
  return (1-t) * x + t * y
)")
             .arg(InterpolateNode::TYPE)},
        {BackendLanguage::GLSL,
         ::transform<QString, QList>(supported_glsl_types,
                                     [](auto&& types) {
                                       const auto& [return_type, arg_type] = types;
                                       return glsl_definition_template
                                           .arg(NodeCompilerGLSL::translate_type(return_type))
                                           .arg(NodeCompilerGLSL::translate_type(arg_type));
                                     })
             .join("\n")
             .arg(InterpolateNode::TYPE)},
    },
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Interpolation")}};

InterpolateNode::InterpolateNode(NodeModel& model) : Node(model)
{
  static constexpr double STEP = 0.01;
  const QString category = tr("Node");
  create_property<FloatProperty>(LEFT_VALUE_KEY, PortType::Input, 0)
      .set_label(QObject::tr("left"))
      .set_category(category);
  create_property<FloatProperty>(RIGHT_VALUE_KEY, PortType::Input, 0)
      .set_label(QObject::tr("right"))
      .set_category(category);
  create_property<FloatProperty>(BALANCE_PROPERTY_KEY, PortType::Input, 0)
      .set_range(0.0, 1.0)
      .set_step(STEP)
      .set_label(QObject::tr("t"))
      .set_category(category);
  const auto linear_spline = SplineType::Initialization::Linear;
  const SplineType default_ramp(linear_spline, false);
  create_property<SplineProperty>(RAMP_PROPERTY_KEY, PortType::Input, default_ramp)
      .set_label(QObject::tr(""))
      .set_category(category);
  m_output = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString InterpolateNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    const QString type_a = find_port<InputPort>(LEFT_VALUE_KEY)->data_type();
    const QString type_b = find_port<InputPort>(RIGHT_VALUE_KEY)->data_type();
    switch (language()) {
    case BackendLanguage::GLSL:
      if (types::is_numeric(type_a) && types::is_numeric(type_b)) {
        return types::FLOAT_TYPE;
      } else if (types::is_vector(type_a) && types::is_vector(type_b)) {
        return types::FLOATVECTOR_TYPE;
      } else if (type_a == types::COLOR_TYPE && type_b == types::COLOR_TYPE) {
        return types::COLOR_TYPE;
      } else {
        return types::INVALID_TYPE;
      }
    case BackendLanguage::Python:
      if (types::is_numeric(type_a) && types::is_numeric(type_b)) {
        return types::FLOAT_TYPE;
      } else if ((types::is_vector(type_a) || types::is_numeric(type_a))
                 && (types::is_vector(type_b) || types::is_numeric(type_b))) {
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

QString InterpolateNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(LEFT_VALUE_KEY), find_port<InputPort>(RIGHT_VALUE_KEY)};
  return fst_con_ptype(ports, types::FLOAT_TYPE);
}

bool InterpolateNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  if (&port == find_port<InputPort>(RAMP_PROPERTY_KEY)
      || &port == find_port<InputPort>(BALANCE_PROPERTY_KEY)) {
    return port.data_type() == type;
  } else {
    switch (language()) {
    case BackendLanguage::Python:
      [[fallthrough]];
    case BackendLanguage::GLSL:
      return true;
    default:
      Q_UNREACHABLE();
      return true;
    }
  }
}

}  // namespace omm::nodes
