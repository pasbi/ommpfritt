#include "nodesystem/nodecompiler.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/port.h"

namespace omm::nodes
{

AbstractNodeCompiler::AbstractNodeCompiler(BackendLanguage language, const NodeModel& model)
    : language(language), m_model(model)
{
}

std::set<Node*> AbstractNodeCompiler::nodes() const
{
  return m_model.nodes();
}

void AbstractNodeCompiler::generate_statements(std::set<QString>& used_node_types,
                                               std::list<std::unique_ptr<Statement>>& statements) const
{
  const auto successors = [](Node* node) {
    std::set<Node*> descendants;
    for (OutputPort* op : node->ports<OutputPort>()) {
      for (InputPort* ip : op->connected_inputs()) {
        descendants.insert(&ip->node);
      }
    }
    return descendants;
  };

  const auto [has_cycle, sequence] = topological_sort<Node*>(nodes(), successors);
  assert(!has_cycle);

  for (Node* node : sequence) {
    used_node_types.insert(node->type());
    statements.push_back(std::make_unique<NodeStatement>(*node));
    for (OutputPort* op : node->ports<OutputPort>()) {
      for (InputPort* ip : op->connected_inputs()) {
        statements.push_back(std::make_unique<ConnectionStatement>(*op, *ip));
      }
    }
  }
}

QString AbstractNodeCompiler::code()
{
  if (m_is_dirty) {
    compile();
  }
  return m_code;
}

QString AbstractNodeCompiler::error()
{
  if (m_is_dirty) {
    compile();
  }
  return m_last_error.message;
}

bool AbstractNodeCompiler::can_cast(const Type from, const Type to)
{
  const auto is_convertible_type = [](const Type t) {
    return is_scalar(t) || is_vector(t) || is_color(t);
  };

  // Scalar, vector and color are all convertable into each other.
  // Other types are not convertable into anything else.
  return is_convertible_type(from) && is_convertible_type(to);
}

void AbstractNodeCompiler::invalidate()
{
  m_is_dirty = true;
}

bool AbstractNodeCompiler::check(const AssemblyError& error)
{
  if (error.message.isEmpty()) {
    return true;
  } else {
    m_last_error = error;
    Q_EMIT compilation_failed(error.message);
    return false;
  }
}

AbstractNodeCompiler::AssemblyError::AssemblyError(const QString& message)
    : message(message)
{
}

}  // namespace omm
