#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/splineproperty.h"

namespace
{
using namespace omm::NodeCompilerTypes;
const std::set<std::array<QString, 2>> supported_glsl_types{{FLOATVECTOR_TYPE, FLOATVECTOR_TYPE},
                                                            {FLOAT_TYPE, INTEGER_TYPE},
                                                            {FLOAT_TYPE, FLOAT_TYPE},
                                                            {
                                                                COLOR_TYPE,
                                                                COLOR_TYPE,
                                                            },
                                                            {FLOATVECTOR_TYPE, INTEGERVECTOR_TYPE}};

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
          .arg(omm::NodeCompilerGLSL::translate_type(SPLINE_TYPE));

}  // namespace

namespace omm
{
using namespace omm::NodeCompilerTypes;
const Node::Detail InterpolateNode::detail{
    {
        {AbstractNodeCompiler::Language::Python,
         QString(R"(
def %1(x, y, balance, ramp):
  t = ramp.value(balance)
  return (1-t) * x + t * y
)")
             .arg(InterpolateNode::TYPE)},
        {AbstractNodeCompiler::Language::GLSL,
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
    {
        QT_TRANSLATE_NOOP("NodeMenuPath", "Interpolation"),
    }};

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
    case AbstractNodeCompiler::Language::GLSL:
      if (is_numeric(type_a) && is_numeric(type_b)) {
        return FLOAT_TYPE;
      } else if (is_vector(type_a) && is_vector(type_b)) {
        return FLOATVECTOR_TYPE;
      } else if (type_a == COLOR_TYPE && type_b == COLOR_TYPE) {
        return COLOR_TYPE;
      } else {
        return INVALID_TYPE;
      }
    case AbstractNodeCompiler::Language::Python:
      if (is_numeric(type_a) && is_numeric(type_b)) {
        return FLOAT_TYPE;
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

QString InterpolateNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(LEFT_VALUE_KEY), find_port<InputPort>(RIGHT_VALUE_KEY)};
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
      [[fallthrough]];
    case AbstractNodeCompiler::Language::GLSL:
      return true;
    default:
      Q_UNREACHABLE();
      return true;
    }
  }
}

}  // namespace omm
