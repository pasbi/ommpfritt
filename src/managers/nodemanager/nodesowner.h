#pragma once

#include <memory>

namespace omm
{

class NodeModel;
class Scene;

class NodesOwner
{
protected:
  explicit NodesOwner(Scene& scene);
public:
  virtual ~NodesOwner();
  NodeModel& node_model() const { return *m_model; }
private:
  std::unique_ptr<NodeModel> m_model;
};

}  // namespace
