#pragma once

#include "common.h"
#include "nodesystem/statement.h"
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
static constexpr auto OPTION_TYPE = "Option";
static constexpr auto FLOATVECTOR_TYPE = "FloatVector";
static constexpr auto INTEGERVECTOR_TYPE = "IntegerVector";
static constexpr auto STRING_TYPE = "String";
static constexpr auto COLOR_TYPE = "Color";
static constexpr auto REFERENCE_TYPE = "Reference";
static constexpr auto BOOL_TYPE = "Bool";
static constexpr auto SPLINE_TYPE = "Spline";

bool is_numeric(const QString& type);
bool is_integral(const QString& type);
bool is_vector(const QString& type);
}  // namespace NodeCompilerTypes

class AbstractNodeCompiler : public QObject
{
  Q_OBJECT
public:
  enum class Language { Python, GLSL };
  static std::set<QString> supported_types(Language language);
  [[nodiscard]] QString last_error() const
  {
    return m_last_error;
  }

protected:
  AbstractNodeCompiler(Language language, const NodeModel& model);
  [[nodiscard]] std::set<Node*> nodes() const;

  void generate_statements(std::set<QString>& used_node_types,
                           std::list<std::unique_ptr<Statement>>& statements) const;

public:
  const Language language;
  [[nodiscard]] const NodeModel& model() const
  {
    return m_model;
  }
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
    std::list<std::unique_ptr<Statement>> statements;
    generate_statements(used_node_types, statements);
    const auto& self = static_cast<const ConcreteCompiler&>(*this);

    const auto check = [this](const QString& msg) {
      if (msg.isEmpty()) {
        return true;
      } else {
        m_last_error = msg;
        Q_EMIT compilation_failed(msg);
        return false;
      }
    };

    if (!check(self.generate_header(lines))) {
      return false;
    }
    for (const QString& type : used_node_types) {
      if (!check(self.define_node(type, lines))) {
        return false;
      }
    }

    if (!check(self.start_program(lines))) {
      return false;
    }

    for (auto&& statement : statements) {
      if (statement->is_connection()) {
        const auto& cs = dynamic_cast<const ConnectionStatement&>(*statement);
        if (!check(self.compile_connection(cs.source, cs.target, lines))) {
          return false;
        }
      } else {
        const auto& ns = dynamic_cast<const NodeStatement&>(*statement);
        if (!check(self.compile_node(ns.node, lines))) {
          return false;
        }
      }
    }
    if (!check(self.end_program(lines))) {
      return false;
    }

    m_code = lines.join("\n");
    Q_EMIT compilation_succeeded(m_code);
    return true;
  }

protected:
  explicit NodeCompiler(const NodeModel& model)
      : AbstractNodeCompiler(ConcreteCompiler::LANGUAGE, model)
  {
  }

  void statements();
};

}  // namespace omm
