#pragma once

#include "common.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <list>
#include <set>

namespace omm
{

class OutputPort;
class InputPort;
class Node;
class AbstractPort;
class NodeModel;

namespace NodeCompilerTypes
{
  static constexpr auto INVALID_TYPE = "Invalid";
  static constexpr auto FLOAT_TYPE = "Float";
  static constexpr auto INTEGER_TYPE = "Integer";
  static constexpr auto OPTIONS_TYPE = "Options";
  static constexpr auto FLOATVECTOR_TYPE = "FloatVector";
  static constexpr auto INTEGERVECTOR_TYPE = "IntegerVector";
  static constexpr auto STRING_TYPE = "String";
  static constexpr auto COLOR_TYPE = "Color";
  static constexpr auto REFERENCE_TYPE = "Reference";
  static constexpr auto BOOL_TYPE = "Bool";

  bool is_numeric(const QString& type);
  bool is_integral(const QString& type);
  bool is_vector(const QString& type);
}

class AbstractNodeCompiler : public QObject
{
  Q_OBJECT
public:
  enum class Language { Python, GLSL };
protected:
  AbstractNodeCompiler(const NodeModel& model);
  std::set<Node*> nodes() const;
  struct Statement
  {
    Statement(const OutputPort& source, const InputPort& target);
    Statement(const Node& node);
    const bool is_connection;
    const OutputPort* const source = nullptr;
    const InputPort* const target = nullptr;
    const Node* const node = nullptr;
    bool operator<(const Statement& other) const;
  private:
    std::set<const AbstractPort*> defines() const;
    std::set<const AbstractPort*> uses() const;
  };
  void generate_statements(std::set<QString>& used_node_types, std::list<Statement>& statements);
  const NodeModel& model() const { return m_model; }
Q_SIGNALS:
  void compilation_succeeded(const QString& code);
private:
  const NodeModel& m_model;
};

template<typename ConcreteCompiler> class NodeCompiler : public AbstractNodeCompiler
{
public:
  QString compile()
  {
    std::set<QString> used_node_types;
    std::list<Statement> statements;
    generate_statements(used_node_types, statements);
    QStringList lines;
    const auto& self = static_cast<const ConcreteCompiler&>(*this);
    lines.push_back(self.header());
    for (const QString& type : used_node_types) {
      lines.push_back(self.define_node(type));
    }

    lines.push_back(self.start_program());
    for (const Statement& statement : statements) {
      if (statement.is_connection) {
        lines.push_back(self.compile_connection(*statement.source, *statement.target));
      } else {
        lines.push_back(self.compile_node(*statement.node));
      }
    }
    lines.push_back(self.end_program());
    const QString code = lines.join("\n");
    Q_EMIT compilation_succeeded(code);
    return code;
  }

protected:
  explicit NodeCompiler(const NodeModel& model)
    : AbstractNodeCompiler(model)
  {}

  void statements();
};

}  // namespace omm
