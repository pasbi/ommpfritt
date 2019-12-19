#include "managers/nodemanager/nodes/mathnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

MathNode::MathNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  create_property<OptionsProperty>(OPERATION_PROPERTY_KEY, 0.0)
      .set_options({ tr("+"), tr("-"), tr("*"), tr("/"), tr("pow") })
      .set_label(QObject::tr("Operation")).set_category(category);
  create_property<FloatProperty>(A_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("a")).set_category(category);
  create_property<FloatProperty>(B_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("b")).set_category(category);
  add_port<OrdinaryPort<PortType::Output>>(tr("result"), "Float");
}

QString MathNode::definition(NodeCompiler::Language language) const
{
  return QString(R"(
def %1(op, a, b):
    def do_op(op, a, b):
        if op == 0:
            return a + b
        elif op == 1:
            return a - b
        elif op == 2:
            return a * b
        elif op == 3:
            return a / b
        elif op == 4:
            return a ** b
        else:
            return 0.0;
    return float(do_op(op, a, b))
)").arg(name(language));
}

QString MathNode::name(NodeCompiler::Language language) const
{
  Q_UNUSED(language)
  return "math";
}

void MathNode::on_property_value_changed(Property* property)
{
}

}  // namespace
