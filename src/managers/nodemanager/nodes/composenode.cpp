#include "managers/nodemanager/nodes/composenode.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

DecomposeNode::DecomposeNode(Scene* scene) : Node(scene)
{
  const QString category = tr("Node");
  create_property<FloatVectorProperty>(INPUT_PROPERTY_KEY, Vec2f(0.0, 0.0))
      .set_label(tr("in")).set_category(category);
  m_output_x_port = &add_port<OrdinaryPort<PortType::Output>>(tr("x"));
  m_output_y_port = &add_port<OrdinaryPort<PortType::Output>>(tr("y"));
}

QString DecomposeNode::definition() const
{
  return QString(R"(
def %1(v):
  return v
)").arg(uuid());
}

QString DecomposeNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_output_x_port || &port == m_output_y_port) {
    const QString type = find_port<InputPort>(*property(INPUT_PROPERTY_KEY))->data_type();
    if (type == INTEGERVECTOR_TYPE) {
      return INTEGER_TYPE;
    } else if (type == FLOATVECTOR_TYPE) {
      return FLOAT_TYPE;
    }
  }
  return INVALID_TYPE;
}

QString DecomposeNode::title() const
{
  return tr("Decompose");
}

bool DecomposeNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  using namespace NodeCompilerTypes;
  Q_UNUSED(port)
  return is_vector(type);
}

ComposeNode::ComposeNode(Scene* scene) : Node(scene)
{
  const QString category = tr("Node");
  create_property<FloatProperty>(INPUT_X_PROPERTY_KEY, 0.0)
      .set_label(tr("x")).set_category(category);
  create_property<FloatProperty>(INPUT_Y_PROPERTY_KEY, 0.0)
      .set_label(tr("y")).set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("vector"));
}

QString ComposeNode::definition() const
{
  return QString(R"(
def %1(a, b):
  return [a, b]
)").arg(uuid());
}

QString ComposeNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_output_port) {
    const QString type_a = find_port<InputPort>(*property(INPUT_X_PROPERTY_KEY))->data_type();
    const QString type_b = find_port<InputPort>(*property(INPUT_Y_PROPERTY_KEY))->data_type();

    if (is_integral(type_a) && is_integral(type_b)) {
      return INTEGERVECTOR_TYPE;
    } else if (is_numeric(type_a) && is_numeric(type_b)) {
      return FLOATVECTOR_TYPE;
    } else {
      return INVALID_TYPE;
    }
  }
  return INVALID_TYPE;
}

QString ComposeNode::title() const
{
  return tr("Compose");
}

bool ComposeNode::accepts_input_data_type(const QString &type, const InputPort &port) const
{
  Q_UNUSED(port)
  return NodeCompilerTypes::is_numeric(type);
}

}  // namespace omm
