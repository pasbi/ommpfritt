#include "nodesystem/nodes/constantnode.h"
#include "managers/propertymanager/userpropertydialog.h"
#include "nodesystem/nodemodel.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"
#include "propertytypeenum.h"
#include "scene/scene.h"
#include <QMenu>

namespace omm::nodes
{

const Node::Detail ConstantNode::detail {
    .definitions = {{BackendLanguage::Python, ""}, {BackendLanguage::GLSL, ""}},
    .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "General")}
};

ConstantNode::ConstantNode(NodeModel& model) : Node(model)
{
}

QString ConstantNode::type() const
{
  return TYPE;
}

void ConstantNode::populate_menu(QMenu& menu)
{
  QAction* edit_port_action = menu.addAction(tr("Edit ports ..."));
  connect(edit_port_action, &QAction::triggered, this, [this, &menu]() {
    const auto types = model().compiler().supported_types();
    const auto type_names = util::transform(types, [](const Type type) {
      return QString{"%1"}.arg(variant_type_name(type).data());
    });
    auto dialog = UserPropertyDialog(*this, type_names, &menu);
    dialog.exec();
  });
}

}  // namespace omm::nodes
