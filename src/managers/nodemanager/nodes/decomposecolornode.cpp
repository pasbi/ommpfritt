#include "managers/nodemanager/nodes/decomposecolornode.h"
#include "properties/floatproperty.h"
#include "properties/colorproperty.h"
#include "managers/nodemanager/ordinaryport.h"

namespace omm
{

const Node::Detail DecomposeColorNode::detail {
  {
    {
      AbstractNodeCompiler::Language::Python,
      QString(R"(
def %1(c):
    return c
)")
    },
    {
      AbstractNodeCompiler::Language::GLSL,
      QString(R"(
float %1_0(vec4 c) { return c.r; }
float %1_1(vec4 c) { return c.g; }
float %1_2(vec4 c) { return c.b; }
float %1_3(vec4 c) { return c.a; }
)").arg(DecomposeColorNode::TYPE)
    }
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "Color"),
  },
};

DecomposeColorNode::DecomposeColorNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  create_property<ColorProperty>(INPUT_PROPERTY_KEY, Color(Color::Model::RGBA, { 0, 0, 0 }, 0))
      .set_label(tr("color")).set_category(category);
  add_port<OrdinaryPort<PortType::Output>>(tr("R"));
  add_port<OrdinaryPort<PortType::Output>>(tr("G"));
  add_port<OrdinaryPort<PortType::Output>>(tr("B"));
  add_port<OrdinaryPort<PortType::Output>>(tr("A"));
}

QString DecomposeColorNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  Q_UNUSED(port)
  return FLOAT_TYPE;
}

QString DecomposeColorNode::title() const
{
  return tr("Decompose Color");
}

bool DecomposeColorNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  using namespace NodeCompilerTypes;
  Q_UNUSED(port)
  return type == COLOR_TYPE;
}

}  // namespace omm
