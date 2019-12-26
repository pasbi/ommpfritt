#pragma once

#include <memory>
#include "managers/nodemanager/nodecompiler.h"
#include "cachedgetter.h"

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

  QString code() const { return m_compiler_cache(); }
  NodeCompiler* compiler() const { return m_compiler_cache.compiler(); }

private:
  std::unique_ptr<NodeModel> m_model;

  class CompilerCache : public CachedGetter<QString, NodesOwner>
  {
  public:
    CompilerCache(NodesOwner& self);
    ~CompilerCache();
    NodeCompiler* compiler() const { return m_compiler.get(); }
  protected:
    QString compute() const override;
    mutable std::unique_ptr<NodeCompiler> m_compiler;
  } m_compiler_cache;
};

}  // namespace
