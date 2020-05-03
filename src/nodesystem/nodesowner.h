#pragma once

#include <memory>
#include "cachedgetter.h"
#include "nodesystem/nodecompiler.h"

namespace omm
{

class Scene;
class Property;
class NodeModel;
class TriggerProperty;

class NodesOwner
{
protected:
  NodesOwner(AbstractNodeCompiler::Language language, Scene& scene);
  NodesOwner(const NodesOwner& other);
  ~NodesOwner();

  void connect_edit_property(TriggerProperty& property, QObject& self);

public:
  NodeModel* node_model() const;

private:
  std::unique_ptr<NodeModel> m_node_model;
};

}  // namespace
