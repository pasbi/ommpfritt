#include "nodesystem/nodecompiler.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/port.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"

namespace omm
{

bool NodeCompilerTypes::is_integral(const QString& type)
{
  return type == BOOL_TYPE || type == INTEGER_TYPE || type == OPTION_TYPE;
}

bool NodeCompilerTypes::is_numeric(const QString& type)
{
  return is_integral(type) || type == FLOAT_TYPE;
}

bool NodeCompilerTypes::is_vector(const QString& type)
{
  return type == INTEGERVECTOR_TYPE || type == FLOATVECTOR_TYPE;
}

std::set<QString> AbstractNodeCompiler::supported_types(AbstractNodeCompiler::Language language)
{
  switch (language) {
  case AbstractNodeCompiler::Language::Python:
    return Property::keys();
  case AbstractNodeCompiler::Language::GLSL:
    return ::filter_if(Property::keys(), [](const auto& c) {
      return c != StringProperty::TYPE() && c != TriggerProperty::TYPE();
    });
  default:
    Q_UNREACHABLE();
    return std::set<QString>();
  }
}

AbstractNodeCompiler::AbstractNodeCompiler(Language language, const NodeModel& model)
    : language(language), m_model(model)
{
}

std::set<Node*> AbstractNodeCompiler::nodes() const
{
  return m_model.nodes();
}

void AbstractNodeCompiler::generate_statements(
    std::set<QString>& used_node_types,
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
  return m_last_error;
}

void AbstractNodeCompiler::invalidate()
{
  m_is_dirty = true;
}

}  // namespace omm
