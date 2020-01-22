#include "managers/nodemanager/nodes/constantnode.h"
#include "properties/stringproperty.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"
#include "managers/propertymanager/userpropertydialog.h"

namespace omm
{

const Node::Detail ConstantNode::detail { {
    { AbstractNodeCompiler::Language::Python, "" },
    { AbstractNodeCompiler::Language::GLSL, "" }
} };

ConstantNode::ConstantNode(NodeModel& model) : Node(model)
{
}

void ConstantNode::populate_menu(QMenu& menu)
{
  QAction* edit_port_action = menu.addAction(tr("Edit ports ..."));
  connect(edit_port_action, &QAction::triggered, [this, &menu]() {
    auto dialog = UserPropertyDialog(*this, { StringProperty::TYPE }, &menu);
    dialog.exec();
  });
}

void ConstantNode::on_property_value_changed(Property* property)
{
}

}  // namespace
