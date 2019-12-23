#pragma once

#include <memory>
#include "managers/nodemanager/nodecompiler.h"

namespace omm
{

class Scene;
class Property;

class NodesOwner
{
protected:
  explicit NodesOwner(NodeCompiler::Language language, Scene& scene);
  NodesOwner(const NodesOwner& other);
  std::unique_ptr<Property> make_edit_nodes_property();
public:
  virtual ~NodesOwner();
  NodeModel& node_model() const { return *m_model; }
private:
  std::unique_ptr<NodeModel> m_model;
};

}  // namespace
