#include "nodesystem/nodes/colorconvertnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/colorproperty.h"
#include "properties/optionproperty.h"

namespace
{

std::deque<QString> conversion_options()
{
  return {
    omm::nodes::ColorConvertNode::tr("Identity"),
    omm::nodes::ColorConvertNode::tr("RGBA -> HSVA"),
    omm::nodes::ColorConvertNode::tr("HSVA -> RGBA"),
  };
}

constexpr auto python_defintion_template = R"(
def %1(option, color):
  if option == 0:
    return color
  elif option == 1:
    return color
  elif option == 2:
    return color
  else:
    // unreachable
    return [ 0.0, 0.0, 0.0, 1.0 ]
)";

constexpr auto glsl_definition_template = R"(
vec4 %1_0(int option, vec4 color) {
  if (option == 0) {
    return color;
  } else if (option == 1) {
    // rgb -> hsv
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(color.bg, K.wz), vec4(color.gb, K.xy), step(color.b, color.g));
    vec4 q = mix(vec4(p.xyw, color.r), vec4(color.r, p.yzx), step(p.x, color.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    vec3 hsv = vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    return vec4(hsv, color.a);
  } else if (option == 2) {
    // hsv -> rgb
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
    vec3 rgb = color.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), color.y);
    return vec4(rgb, color.a);
  } else {
    // unreachable
    return vec4(0.0, 0.0, 0.0, 1.0);
  }
})";

}  // namespace

namespace omm::nodes
{

const Node::Detail ColorConvertNode::detail {
  .definitions = {
        {BackendLanguage::Python, QString{python_defintion_template}.arg(TYPE)},
        {BackendLanguage::GLSL, QString{glsl_definition_template}.arg(ColorConvertNode::TYPE)}
  },
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Color")},
};

ColorConvertNode::ColorConvertNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<OptionProperty>(CONVERSION_PROPERTY_KEY)
      .set_options(conversion_options())
      .set_label(tr("conversion"))
      .set_category(category);
  create_property<ColorProperty>(COLOR_PROPERTY_KEY, Color())
      .set_label(tr("color"))
      .set_category(category);
  m_vector_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("color"));
}

Type ColorConvertNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_vector_output_port) {
    return Type::Color;
  }
  return Type::Invalid;
}

QString ColorConvertNode::title() const
{
  return tr("Convert");
}

QString ColorConvertNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
