#include "managers/nodemanager/nodesowner.h"
#include "managers/nodemanager/nodemodel.h"
#include "properties/triggerproperty.h"
#include "managers/nodemanager/nodemanager.h"
#include "mainwindow/application.h"

namespace omm
{

NodesOwner::NodesOwner(NodeCompiler::Language language, Scene& scene)
  : m_model(std::make_unique<NodeModel>(language, scene))
{
}

NodesOwner::NodesOwner(const NodesOwner& other)
  : m_model(std::make_unique<NodeModel>(*other.m_model))
{
}

std::unique_ptr<Property> NodesOwner::make_edit_nodes_property()
{
  auto property = std::make_unique<TriggerProperty>();
  QObject::connect(property.get(), &Property::value_changed, property.get(), [this]() {
    Manager& manager = Application::instance().get_active_manager(NodeManager::TYPE);
    static_cast<NodeManager&>(manager).set_model(&node_model());
  });
  return property;
}

NodesOwner::~NodesOwner()
{

}

}  // namespace omm
