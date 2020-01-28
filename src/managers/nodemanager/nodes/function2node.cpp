#include "managers/nodemanager/nodes/function2node.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/optionsproperty.h"

namespace omm
{

const Node::Detail Function2Node::detail { {
    { AbstractNodeCompiler::Language::Python, QString(R"(
import math
def %1(op, x, y):
  if op == 0:
    return math.atan2(y, x)
  elif op == 1:
    return math.sqrt(x**2.0 + y**2.0)
  elif op == 2:
    return math.pow(x, y)
  elif op == 3:
    return math.min(x, y)
  elif (op == 4:
    return math.max(x, y)
  else:
    return 0.0
)").arg(Function2Node::TYPE) },
    { AbstractNodeCompiler::Language::GLSL, QString(R"(
float %1_0(int op, float x, float y) {
  if (op == 0) {
    return atan(y, x);
  } else if (op == 1) {
    return length(vec2(x, y));
  } else if (op == 2) {
    return pow(x, y);
  } else if (op == 3) {
    return min(x, y);
  } else if (op == 4) {
    return max(x, y);
  } else {
    return 0.0;
  }
}
float %1_0(int op, int x, int y) {
  return %1_0(op, float(x), float(y));
}
)").arg(Function2Node::TYPE) }
    } };

Function2Node::Function2Node(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<OptionsProperty>(OPERATION_PROPERTY_KEY, 0)
      .set_options({ tr("atan2"), tr("length"), tr("pow"), tr("min"), tr("max") })
      .set_label(QObject::tr("Operation")).set_category(category);
  create_property<FloatProperty>(INPUT_A_PROPERTY_KEY, 0.0)
      .set_label(tr("a")).set_category(category);
  create_property<FloatProperty>(INPUT_B_PROPERTY_KEY, 0.0)
      .set_label(tr("b")).set_category(category);
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString Function2Node::output_data_type(const OutputPort& port) const
{
  Q_UNUSED(port)
  using namespace NodeCompilerTypes;
  return FLOAT_TYPE;
}

bool Function2Node::accepts_input_data_type(const QString &type, const InputPort &port) const
{
  Q_UNUSED(port)
  return NodeCompilerTypes::is_numeric(type);
}

}  // namespace omm
