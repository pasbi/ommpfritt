#include "nodesystem/nodes/linepatternnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"

namespace omm::nodes
{

const Node::Detail LinePatternNode::detail{
    .definitions = {{BackendLanguage::GLSL,
      QString(R"(
float %1_0(float frequency, float ratio, float left_ramp, float right_ramp, float v) {
  float lambda = 1.0 / frequency;
  v = mod(clamp(0.0, 1.0, v), lambda) * frequency;
  left_ramp = left_ramp * ratio;
  right_ramp = right_ramp * (1-ratio);

  if (v > 1.0 - right_ramp) {
    return (1.0 - v) / right_ramp;
  } else if (v > ratio) {
    return 1.0;
  } else if (v > ratio - left_ramp) {
    return (v - ratio + left_ramp) / left_ramp;
  } else {
    return 0.0;
  }
}
)")
          .arg(LinePatternNode::TYPE)}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Pattern")},
};

LinePatternNode::LinePatternNode(NodeModel& model) : Node(model)
{
  static constexpr double STEP = 0.01;
  static constexpr double HALF = 0.5;
  const auto category = tr("Lines");
  create_property<FloatProperty>(FREQUENCY_PROPERTY_KEY, PortType::Input, 1.0)
      .set_label(tr("freq"))
      .set_category(category);
  create_property<FloatProperty>(RATIO_PROPERTY_KEY, PortType::Input, HALF)
      .set_range(0.0, 1.0)
      .set_step(STEP)
      .set_label(tr("ratio"))
      .set_category(category);
  create_property<FloatProperty>(LEFT_RAMP_PROPERTY_KEY, PortType::Input, 0.0)
      .set_range(0.0, 1.0)
      .set_step(STEP)
      .set_label(tr("left ramp"))
      .set_category(category);
  create_property<FloatProperty>(RIGHT_RAMP_PROPERTY_KEY, PortType::Input, 0.0)
      .set_range(0.0, 1.0)
      .set_step(STEP)
      .set_label(tr("right ramp"))
      .set_category(category);
  m_position_port = &add_port<OrdinaryPort<PortType::Input>>(tr("position"));
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("lines"));
}

QString LinePatternNode::type() const
{
  return TYPE;
}

Type LinePatternNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output_port) {
    return Type::Float;
  } else {
    return Type::Invalid;
  }
}

Type LinePatternNode::input_data_type(const InputPort& port) const
{
  if (port.flavor == PortFlavor::Property) {
    return dynamic_cast<const PropertyPort<PortType::Input>&>(port).data_type();
  } else if (&port == m_position_port) {
    return Type::Float;
  } else {
    return Type::Invalid;
  }
}

QString LinePatternNode::title() const
{
  return tr("Line Pattern");
}

}  // namespace omm::nodes
