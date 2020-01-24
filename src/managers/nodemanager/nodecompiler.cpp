#include "managers/nodemanager/nodecompiler.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodemodel.h"
#include <sstream>

namespace omm
{

bool NodeCompilerTypes::is_integral(const QString& type)
{
  return type == BOOL_TYPE || type == INTEGER_TYPE || type == OPTIONS_TYPE;
}

bool NodeCompilerTypes::is_numeric(const QString& type)
{
  return is_integral(type) || type == FLOAT_TYPE;
}

bool NodeCompilerTypes::is_vector(const QString& type)
{
  return type == INTEGERVECTOR_TYPE || type == FLOATVECTOR_TYPE;
}

AbstractNodeCompiler::Statement::Statement(const OutputPort& source, const InputPort& target)
  : is_connection(true), source(&source), target(&target), node(nullptr)
{
}

AbstractNodeCompiler::Statement::Statement(const Node& node)
  : is_connection(false), source(nullptr), target(nullptr), node(&node)
{

}

bool AbstractNodeCompiler::Statement::operator<(const AbstractNodeCompiler::Statement& other) const
{
  static constexpr auto dont_care = false;  // could be true as well, value does not matter.

  static const auto intersect = [](const auto& set_a, const auto& set_b) {
    static_assert(std::is_same_v<std::decay_t<decltype(set_a)>, std::decay_t<decltype(set_b)>>);
    using T = std::decay_t<decltype(set_a)>;
    T intersection;
    std::set_intersection(set_a.begin(), set_a.end(),
                          set_b.begin(), set_b.end(),
                          std::inserter(intersection, intersection.begin()));
    return intersection;
  };

  const bool smaller = !intersect(this->defines(), other.uses()).empty();
  const bool greater = !intersect(other.defines(), this->uses()).empty();
  const bool conflict = !intersect(other.defines(), this->defines()).empty();
  if (smaller && greater) {
    LERROR << "dependency cycle!";
    return dont_care;
  } else if (conflict) {
    LERROR << "multiple declaration!";
    return dont_care;
  } else {
    return smaller;
  }
}

std::set<const AbstractPort*> AbstractNodeCompiler::Statement::defines() const
{
  if (is_connection) {
    return { target };
  } else {
    return ::transform<const AbstractPort*>(node->ports<OutputPort>(), ::identity);
  }
}

std::set<const AbstractPort*> AbstractNodeCompiler::Statement::uses() const
{
  if (is_connection) {
    return { source };
  } else {
    return ::transform<const AbstractPort*>(node->ports<InputPort>(), ::identity);
  }
}

AbstractNodeCompiler::AbstractNodeCompiler(const NodeModel& model) : m_model(model)
{
}

std::set<Node*> AbstractNodeCompiler::nodes() const
{
  return m_model.nodes();
}

void AbstractNodeCompiler::
generate_statements(std::set<QString>& used_node_types, std::list<Statement>& statements)
{
  static const auto is_terminal = [](Node* node) {
    const auto output_ports = node->ports<OutputPort>();
    return std::all_of(output_ports.begin(), output_ports.end(), [](OutputPort* op) {
      return !op->is_connected();
    });
  };
  std::list<Node*> todo = ::transform<Node*, std::list>(::filter_if(nodes(), is_terminal));
  std::set<Node*> done;

  statements.clear();
  used_node_types.clear();

  while (!todo.empty()) {
    Node* node = todo.front();
    used_node_types.insert(node->type());
    todo.pop_front();
    if (!::contains(done, node)) {
      statements.emplace_front(*node);
      for (const InputPort* ip : node->ports<InputPort>()) {
        if (const OutputPort* op = ip->connected_output(); op != nullptr) {
          statements.emplace_front(*op, *ip);
          todo.push_back(&op->node);
        }
      }
      done.insert(node);
    }
  }

  statements.sort();
}

}  // namespace omm
