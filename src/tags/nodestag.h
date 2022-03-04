#pragma once

#include "nodesystem/nodesowner.h"
#include "tags/tag.h"
#include <Qt>
#include <memory>

namespace omm
{

namespace nodes
{
class Node;
class NodeModel;
}  // namespace nodes

class NodesTag
    : public Tag
    , public nodes::NodesOwner
{
  Q_OBJECT
public:
  explicit NodesTag(Object& owner);
  NodesTag(const NodesTag& other);
  ~NodesTag() override;
  NodesTag(NodesTag&&) = delete;
  NodesTag& operator=(NodesTag&&) = delete;
  NodesTag& operator=(const NodesTag&) = delete;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "NodesTag");
  static constexpr auto UPDATE_MODE_PROPERTY_KEY = "update";
  static constexpr auto TRIGGER_UPDATE_PROPERTY_KEY = "trigger";
  static constexpr auto EDIT_NODES_PROPERTY_KEY = "edit-nodes";
  static constexpr auto NODES_POINTER = "nodes";

  void on_property_value_changed(Property* property) override;
  void evaluate() override;
  void force_evaluate() override;
  Flag flags() const override;
  std::set<nodes::Node*> nodes() const;

  void serialize(serialization::SerializerWorker& worker) const override;
  void deserialize(serialization::DeserializerWorker& worker) override;

private:
  void polish();
};

}  // namespace omm
