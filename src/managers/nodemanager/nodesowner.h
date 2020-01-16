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
  explicit NodesOwner(bool evaluate_lazy, NodeCompiler::Language language, Scene& scene);
  NodesOwner(const NodesOwner& other);
  std::unique_ptr<Property> make_edit_nodes_property();
public:
  virtual ~NodesOwner();
  NodeModel& node_model() const { return *m_model; }

  QString code() const { return m_compiler_cache(); }
  NodeCompiler* compiler() const { return m_compiler_cache.compiler(); }
  void set_on_compilation_successful_cb(const std::function<void(const QString&)>& cb);

private:
  std::unique_ptr<NodeModel> m_model;
  std::function<void(const QString&)> m_on_compilation_successful = [](const QString&) {};

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

  friend class CompilerCache;
  const bool m_evaluate_lazy;
};

}  // namespace
