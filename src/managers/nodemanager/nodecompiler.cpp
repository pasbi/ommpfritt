#include "managers/nodemanager/nodecompiler.h"
#include "properties/stringproperty.h"
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

std::ostream& operator<<(std::ostream& ostream, const AbstractNodeCompiler::Statement& statement)
{
  const auto format = [](const auto& set) -> QStringList {
    return ::transform<QString, QList>(set, [](const auto* port) { return port->uuid(); });
  };
  ostream << QString("%1[%2 <= %3]")
      .arg(statement.is_connection ? "connection" : "node")
      .arg(format(statement.defines()).join(", "))
      .arg(format(statement.uses()).join(", ")).toStdString();
  return ostream;
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

const std::set<QString> AbstractNodeCompiler::supported_types(AbstractNodeCompiler::Language language)
{
  switch (language) {
  case AbstractNodeCompiler::Language::Python:
    return Property::keys();
  case AbstractNodeCompiler::Language::GLSL:
    return ::filter_if(Property::keys(), [](const auto& c) {
      return c != StringProperty::TYPE;
    });
  default:
    Q_UNREACHABLE();
    return std::set<QString>();
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
  const auto successors =  [](Node* node) {
    std::set<Node*> descendants;
    for (OutputPort* op : node->ports<OutputPort>()) {
      for (InputPort* ip : op->connected_inputs()) {
        descendants.insert(&ip->node);
      }
    }
    return descendants;
  };

  const auto [ has_cycle, sequence ] = topological_sort<Node*>(nodes(), successors);
  assert(!has_cycle);

  for (Node* node : sequence) {
    used_node_types.insert(node->type());
    statements.emplace_back(*node);
    for (OutputPort* op : node->ports<OutputPort>()) {
      for (InputPort* ip : op->connected_inputs()) {
        statements.emplace_back(*op, *ip);
      }
    }
  }

}


}  // namespace omm
