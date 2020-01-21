#include "managers/nodemanager/nodesowner.h"
#include "managers/nodemanager/nodecompilerglsl.h"
#include "managers/nodemanager/nodecompilerpython.h"
#include "managers/nodemanager/nodemodel.h"
#include "properties/triggerproperty.h"
#include "managers/nodemanager/nodemanager.h"
#include "mainwindow/application.h"

namespace omm
{

AbstractNodesOwner::AbstractNodesOwner(AbstractNodeCompiler::Language language, Scene& scene)
  : m_model(std::make_unique<NodeModel>(language, scene))
{
}

AbstractNodesOwner::AbstractNodesOwner(const AbstractNodesOwner& other)
  : m_model(std::make_unique<NodeModel>(*other.m_model))
{
}

std::unique_ptr<Property> AbstractNodesOwner::make_edit_nodes_property()
{
  auto property = std::make_unique<TriggerProperty>();
  QObject::connect(property.get(), &Property::value_changed, property.get(), [this]() {
    Manager& manager = Application::instance().get_active_manager(NodeManager::TYPE);
    static_cast<NodeManager&>(manager).set_model(&node_model());
  });
  return property;
}

template<typename ConcreteCompiler> NodesOwner<ConcreteCompiler>
::NodesOwner(Scene& scene)
  : AbstractNodesOwner(ConcreteCompiler::language, scene)
  , m_compiler_cache(*this)
{
}

template<typename ConcreteCompiler> NodesOwner<ConcreteCompiler>
::NodesOwner(const NodesOwner& other)
  : AbstractNodesOwner(other)
  , m_compiler_cache(*this)
{
}

template<typename ConcreteCompiler> NodesOwner<ConcreteCompiler>::~NodesOwner()
{
}

template<typename ConcreteCompiler> NodesOwner<ConcreteCompiler>
::CompilerCache::CompilerCache(const omm::NodesOwner<ConcreteCompiler>& self)
  : CachedGetter<QString, NodesOwner>(self)
  , m_compiler(std::make_unique<ConcreteCompiler>(self.node_model()))
{
  NodeModel::connect(&self.node_model(), &NodeModel::topology_changed, [this]() {
    this->invalidate();
    if (!ConcreteCompiler::lazy) {
      // if lazy, wait until compute is called elsewhere.
      compute();
    }
  });
}

template<typename ConcreteCompiler> NodesOwner<ConcreteCompiler>::CompilerCache::~CompilerCache()
{
}

template class NodesOwner<NodeCompilerPython>;
template class NodesOwner<NodeCompilerGLSL>;

}  // namespace omm
