#include "nodesystem/nodes/colorconvertnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/colorproperty.h"
#include "properties/optionproperty.h"

namespace
{
const std::vector<QString> conversion_options{
    omm::ColorConvertNode::tr("Identity"),
    omm::ColorConvertNode::tr("RGBA -> HSVA"),
    omm::ColorConvertNode::tr("HSVA -> RGBA"),
};

}

namespace omm
{
const Node::Detail ColorConvertNode::detail{
    {{AbstractNodeCompiler::Language::Python,
      QString(R"(
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
)")
          .arg(ColorConvertNode::TYPE)},
     {AbstractNodeCompiler::Language::GLSL,
      QString(R"(
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
})")
          .arg(ColorConvertNode::TYPE)}},
    {
        QT_TRANSLATE_NOOP("NodeMenuPath", "Color"),
    },

};

ColorConvertNode::ColorConvertNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<OptionProperty>(CONVERSION_PROPERTY_KEY)
      .set_options(conversion_options)
      .set_label(tr("conversion"))
      .set_category(category);
  create_property<ColorProperty>(COLOR_PROPERTY_KEY, Color())
      .set_label(tr("color"))
      .set_category(category);
  m_vector_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("color"));
}

QString ColorConvertNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_vector_output_port) {
    return COLOR_TYPE;
  }
  return INVALID_TYPE;
}

QString ColorConvertNode::title() const
{
  return tr("Convert");
}

}  // namespace omm
