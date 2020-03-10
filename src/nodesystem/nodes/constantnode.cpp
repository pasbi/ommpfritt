#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodemodel.h"
#include "properties/stringproperty.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"
#include "managers/propertymanager/userpropertydialog.h"

namespace omm
{

const Node::Detail ConstantNode::detail {
  {
    { AbstractNodeCompiler::Language::Python, "" },
    { AbstractNodeCompiler::Language::GLSL, "" }
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "General")
  }
};

ConstantNode::ConstantNode(NodeModel& model) : Node(model)
{
}

void ConstantNode::populate_menu(QMenu& menu)
{
  QAction* edit_port_action = menu.addAction(tr("Edit ports ..."));
  connect(edit_port_action, &QAction::triggered, [this, &menu]() {
    const std::set<QString> types = AbstractNodeCompiler::supported_types(model().language());
    auto dialog = UserPropertyDialog(*this, types, &menu);
    dialog.exec();
  });
}

void ConstantNode::on_property_value_changed(Property* property)
{
}

}  // namespace
