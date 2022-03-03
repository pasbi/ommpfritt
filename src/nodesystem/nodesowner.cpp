#include "nodesystem/nodesowner.h"
#include "main/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "nodesystem/nodemodel.h"
#include "properties/triggerproperty.h"

namespace omm::nodes
{

NodesOwner::NodesOwner(BackendLanguage language, Scene* scene)
    : m_node_model(std::make_unique<NodeModel>(language, scene))
{
}

NodesOwner::NodesOwner(const NodesOwner& other)
    : m_node_model(std::make_unique<NodeModel>(other.node_model()))
{
}

NodesOwner::~NodesOwner() = default;

void NodesOwner::connect_edit_property(TriggerProperty& property, QObject& self)
{
  QObject::connect(&property, &Property::value_changed, &self, [this]() {
    Manager& manager = Application::instance().get_active_manager(NodeManager::TYPE);
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    if (m_node_model) {
      QTimer::singleShot(1, &manager, [this, &manager]() {
        dynamic_cast<NodeManager&>(manager).set_model(m_node_model.get());
      });
    }
  });
}

NodeModel& NodesOwner::node_model() const
{
  return *m_node_model;
}

}  // namespace omm::nodes
