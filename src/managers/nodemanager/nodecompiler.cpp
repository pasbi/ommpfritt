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
    statements.emplace_front(*node);
    for (const InputPort* ip : node->ports<InputPort>()) {
      if (const OutputPort* op = ip->connected_output(); op != nullptr) {
        statements.emplace_front(*op, *ip);
        if (!::contains(done, &op->node)) {
          todo.push_back(&op->node);
        }
      }
    }
    done.insert(node);
  }
}

}  // namespace omm
