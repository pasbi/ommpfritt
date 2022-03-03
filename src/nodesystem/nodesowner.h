#pragma once

#include "cachedgetter.h"
#include "nodesystem/common.h"
#include <memory>

class QObject;

namespace omm
{

class Scene;
class Property;
class TriggerProperty;

}  // namespace omm

namespace omm::nodes
{

class NodeModel;

class NodesOwner
{
protected:
  NodesOwner(BackendLanguage language, Scene* scene);
  NodesOwner(const NodesOwner& other);
  ~NodesOwner();
  void connect_edit_property(TriggerProperty& property, QObject& self);

public:
  NodesOwner(NodesOwner&& other) = delete;
  NodesOwner& operator=(NodesOwner&& other) = delete;
  NodesOwner& operator=(const NodesOwner& other) = delete;

  [[nodiscard]] NodeModel& node_model() const;

private:
  std::unique_ptr<NodeModel> m_node_model;
};

}  // namespace omm::nodes
