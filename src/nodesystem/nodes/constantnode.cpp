#include "nodesystem/nodes/constantnode.h"
#include "managers/propertymanager/userpropertydialog.h"
#include "nodesystem/nodemodel.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"
#include "scene/scene.h"
#include "variant.h"
#include <QMenu>

namespace omm::nodes
{

const Node::Detail ConstantNode::detail{
    .definitions = {{BackendLanguage::Python, ""}, {BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "General")}};

ConstantNode::ConstantNode(NodeModel& model) : Node(model)
{
}

void ConstantNode::populate_menu(QMenu& menu)
{
  QAction* edit_port_action = menu.addAction(tr("Edit ports ..."));
  connect(edit_port_action, &QAction::triggered, this, [this, &menu]() {
    const std::set<QString> types = types::supported_types(model().language());
    auto dialog = UserPropertyDialog(*this, types, &menu);
    dialog.exec();
  });
}

}  // namespace omm::nodes
