#include "nodesystem/nodes/composenode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"

namespace omm::nodes
{

const Node::Detail ComposeNode::detail{
    .definitions = {{BackendLanguage::Python,
      QString(R"(
def %1(a, b):
  return [a, b]
)")
          .arg(ComposeNode::TYPE)},
     {BackendLanguage::GLSL,
      QString(R"(
vec2 %1_0(float a, float b) { return vec2(a, b); }
)")
          .arg(ComposeNode::TYPE)}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Vector")},
};

ComposeNode::ComposeNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(INPUT_X_PROPERTY_KEY, 0.0)
      .set_label(tr("x"))
      .set_category(category);
  create_property<FloatProperty>(INPUT_Y_PROPERTY_KEY, 0.0)
      .set_label(tr("y"))
      .set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("vector"));
}

QString ComposeNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_output_port) {
    const QString type_a = find_port<InputPort>(*property(INPUT_X_PROPERTY_KEY))->data_type();
    const QString type_b = find_port<InputPort>(*property(INPUT_Y_PROPERTY_KEY))->data_type();

    if (types::is_integral(type_a) && types::is_integral(type_b)) {
      return types::INTEGERVECTOR_TYPE;
    } else if (types::is_numeric(type_a) && types::is_numeric(type_b)) {
      return types::FLOATVECTOR_TYPE;
    } else {
      return types::INVALID_TYPE;
    }
  }
  return types::INVALID_TYPE;
}

QString ComposeNode::title() const
{
  return tr("Compose");
}

bool ComposeNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return types::is_numeric(type);
}

}  // namespace omm::nodes
