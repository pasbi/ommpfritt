#pragma once

#include <memory>
#include "tags/tag.h"
#include "cachedgetter.h"
#include <Qt>
#include "managers/nodemanager/nodesowner.h"

namespace omm
{

class NodeModel;
class NodeCompiler;

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
  static constexpr auto NODES_POINTER = "nodes";

  std::unique_ptr<Tag> clone() const override;
  void on_property_value_changed(Property* property) override;
  void evaluate() override;
  void force_evaluate();
  Flag flags() const override;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

private:
  class CompilerCache : public CachedGetter<QString, NodesTag>
  {
  public:
    CompilerCache(NodesTag& self);
    ~CompilerCache();
    NodeCompiler* compiler() const { return m_compiler.get(); }
  protected:
    QString compute() const override;
    mutable std::unique_ptr<NodeCompiler> m_compiler;
  } m_compiler_cache;
};

}  // namespace omm
