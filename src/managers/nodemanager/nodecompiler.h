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
  static const std::set<QString> supported_types(Language language);
  QString last_error() const { return m_last_error; }

protected:
  AbstractNodeCompiler(Language language, const NodeModel& model);
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
    friend std::ostream& operator<<(std::ostream& ostream, const Statement& statement);

  private:
    std::set<const AbstractPort*> defines() const;
    std::set<const AbstractPort*> uses() const;
  };

  friend std::ostream& operator<<(std::ostream& ostream, const omm::AbstractNodeCompiler::Statement& statement);
  void generate_statements(std::set<QString>& used_node_types, std::list<Statement>& statements) const;

public:
  const Language language;
  const NodeModel& model() const { return m_model; }
  QString code();
  QString error();
  virtual bool compile() = 0;

Q_SIGNALS:
  void compilation_succeeded(const QString& code);
  void compilation_failed(const QString& reason);

public Q_SLOTS:
  virtual void invalidate();

protected:
  QString m_last_error = "";
  QString m_code = "";
  bool m_is_dirty = true;

private:
  const NodeModel& m_model;
};

template<typename ConcreteCompiler> class NodeCompiler : public AbstractNodeCompiler
{
public:
  bool compile() override
  {
    m_is_dirty = false;
    m_code = "";
    m_last_error = "";
    QStringList lines;
    std::set<QString> used_node_types;
    std::list<Statement> statements;
    generate_statements(used_node_types, statements);
    const auto& self = static_cast<const ConcreteCompiler&>(*this);

#define CHECK(statement) \
  if (const QString msg = statement; !msg.isEmpty()) { \
    m_last_error = msg; \
    Q_EMIT compilation_failed(msg); \
    return false; \
  }

    CHECK(self.generate_header(lines))
    for (const QString& type : used_node_types) {
      CHECK(self.define_node(type, lines));
    }

    CHECK(self.start_program(lines));

    for (const Statement& statement : statements) {
      if (statement.is_connection) {
        CHECK(self.compile_connection(*statement.source, *statement.target, lines))
      } else {
        CHECK(self.compile_node(*statement.node, lines))
      }
    }
    CHECK(self.end_program(lines));

#undef CHECK

    m_code = lines.join("\n");
    Q_EMIT compilation_succeeded(m_code);
    return true;
  }

protected:
  explicit NodeCompiler(const NodeModel& model)
    : AbstractNodeCompiler(ConcreteCompiler::LANGUAGE, model)
  {}

  void statements();
};

}  // namespace omm
