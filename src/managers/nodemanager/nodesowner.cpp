#include "managers/nodemanager/nodesowner.h"
#include "managers/nodemanager/nodemodel.h"
#include "properties/triggerproperty.h"
#include "managers/nodemanager/nodemanager.h"
#include "mainwindow/application.h"

namespace omm
{

NodesOwner::NodesOwner(NodeCompiler::Language language, Scene& scene)
  : m_model(std::make_unique<NodeModel>(language, scene))
  , m_compiler_cache(*this)
{
}

NodesOwner::NodesOwner(const NodesOwner& other)
  : m_model(std::make_unique<NodeModel>(*other.m_model))
  , m_compiler_cache(*this)
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

NodesOwner::CompilerCache::CompilerCache(omm::NodesOwner& self) : CachedGetter<QString, NodesOwner>(self)
{
  NodeModel::connect(&self.node_model(), &NodeModel::topology_changed, [this]() {
    invalidate();
  });
}

NodesOwner::CompilerCache::~CompilerCache()
{

}

QString NodesOwner::CompilerCache::compute() const
{
  m_compiler = std::make_unique<NodeCompiler>(NodeCompiler::Language::Python);
  m_compiler->compile(m_self.node_model());
  return m_compiler->compilation();
}

}  // namespace omm
