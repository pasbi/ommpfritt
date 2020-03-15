#pragma once

#include <memory>
#include "tags/tag.h"
#include <Qt>
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodesowner.h"
#include "nodesystem/nodecompilerpython.h"

namespace omm
{

class NodeModel;

class NodesTag : public Tag, public NodesOwner
{
  Q_OBJECT
public:
  explicit NodesTag(Object& owner);
  NodesTag(const NodesTag& other);
  ~NodesTag();
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
  std::set<Node*> nodes() const;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
};

}  // namespace omm
