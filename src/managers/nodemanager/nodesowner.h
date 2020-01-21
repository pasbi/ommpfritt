#pragma once

#include <memory>
#include "managers/nodemanager/nodecompiler.h"
#include "cachedgetter.h"

namespace omm
{

class Scene;
class Property;

class AbstractNodesOwner
{
public:
  NodeModel& node_model() const { return *m_model; }

protected:
  AbstractNodesOwner(AbstractNodeCompiler::Language language, Scene& scene);
  AbstractNodesOwner(const AbstractNodesOwner& other);
  std::unique_ptr<Property> make_edit_nodes_property();

private:
  std::unique_ptr<NodeModel> m_model;
};

template<typename ConcreteCompiler> class NodesOwner : public AbstractNodesOwner
{
protected:
  explicit NodesOwner(Scene& scene);
  NodesOwner(const NodesOwner& other);
public:
  virtual ~NodesOwner();

  QString code() const { return m_compiler_cache(); }
  ConcreteCompiler* compiler() const { return m_compiler_cache.compiler(); }

private:
  class CompilerCache : public CachedGetter<QString, NodesOwner>
  {
  public:
    CompilerCache(const NodesOwner& self);
    ~CompilerCache();
    ConcreteCompiler* compiler() const { return m_compiler.get(); }

  protected:
    QString compute() const override { return m_compiler->compile(); }
    mutable std::unique_ptr<ConcreteCompiler> m_compiler;
  } m_compiler_cache;

  friend class CompilerCache;
};

}  // namespace
