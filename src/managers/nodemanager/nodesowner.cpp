#include "managers/nodemanager/nodesowner.h"
#include "managers/nodemanager/nodemodel.h"

namespace omm
{

NodesOwner::NodesOwner(Scene& scene) : m_model(std::make_unique<NodeModel>(scene))
{

}

NodesOwner::~NodesOwner()
{

}

}  // namespace omm
