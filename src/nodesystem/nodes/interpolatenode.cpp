#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/splineproperty.h"

namespace
{

constexpr auto python_definition_template = R"(
def %1(x, y, balance, ramp):
  t = ramp.value(balance)
  return (1-t) * x + t * y
)";

constexpr auto glsl_definition_template = R"(
%3 %1_0(%4 a, %4 b, float t, %2 spline) {
  const int n = SPLINE_SIZE - 1;
  int k = int(t * n);
  int i = clamp(k, 0, n);
  int j = clamp(i+1, 0, n);
  float r = t * n - k;
  float s = mix(spline[i], spline[j], r);
  return mix(a, b, s);
}
)";

struct Overload
{
  omm::Type return_type;
  omm::Type argument_type;

  [[nodiscard]] auto types() const
  {
    return std::array{return_type, argument_type};
  }
};

template<typename Overload>
QString generate_overload(QString template_definition, const Overload& overload)
{
  for (const auto& type : overload.types()) {
    const auto type_name = omm::nodes::NodeCompilerGLSL::type_name(type);
    template_definition = template_definition.arg(type_name);
  }
  return template_definition;
}

constexpr std::array supported_overloads {
  Overload{.return_type = omm::Type::FloatVector, .argument_type = omm::Type::FloatVector},
  Overload{.return_type = omm::Type::Float,       .argument_type = omm::Type::Integer},
  Overload{.return_type = omm::Type::Float,       .argument_type = omm::Type::Float},
  Overload{.return_type = omm::Type::Color,       .argument_type = omm::Type::Color},
  Overload{.return_type = omm::Type::FloatVector, .argument_type = omm::Type::IntegerVector}
};

template<typename Overloads>
QString overload(const QString& definition_template, const Overloads& overloads)
{
  QStringList definitions;
  definitions.reserve(overloads.size());
  for (const auto& overload : overloads) {
    definitions.push_back(generate_overload(definition_template, overload));
  }
  return definitions.join("\n");
}

}  // namespace

namespace omm::nodes
{

const Node::Detail InterpolateNode::detail{
    .definitions = {
        {BackendLanguage::Python, QString{python_definition_template}.arg(InterpolateNode::TYPE)},
        {BackendLanguage::GLSL, overload(QString{glsl_definition_template}
                                          .arg(InterpolateNode::TYPE,
                                               NodeCompilerGLSL::type_name(Type::Spline)),
                                         supported_overloads)
        }
    },

    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Interpolation")}
  };

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

QString InterpolateNode::type() const
{
  return TYPE;
}

Type InterpolateNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output) {
    const auto type_a = find_port<InputPort>(LEFT_VALUE_KEY)->data_type();
    const auto type_b = find_port<InputPort>(RIGHT_VALUE_KEY)->data_type();
    switch (language()) {
    case BackendLanguage::GLSL:
      if (is_numeric(type_a) && is_numeric(type_b)) {
        return Type::Float;
      } else if (is_vector(type_a) && is_vector(type_b)) {
        return Type::FloatVector;
      } else if (type_a == Type::Color && type_b == Type::Color) {
        return Type::Color;
      } else {
        return Type::Invalid;
      }
    case BackendLanguage::Python:
      if (is_numeric(type_a) && is_numeric(type_b)) {
        return Type::Float;
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

Type InterpolateNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  const auto ports
      = std::vector{find_port<InputPort>(LEFT_VALUE_KEY), find_port<InputPort>(RIGHT_VALUE_KEY)};
  return fst_con_ptype(ports, Type::Float);
}

bool InterpolateNode::accepts_input_data_type(const Type type, const InputPort& port, bool with_cast) const
{
  Q_UNUSED(with_cast)
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
