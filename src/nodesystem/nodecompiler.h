#pragma once

#include "common.h"
#include "nodesystem/statement.h"
#include "propertytypeenum.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <list>
#include <set>
#include <memory>

namespace omm::nodes
{

class OutputPort;
class InputPort;
class Node;
class AbstractPort;
class NodeModel;

class AbstractNodeCompiler : public QObject
{
  Q_OBJECT
public:
  [[nodiscard]] QString last_error() const
  {
    return m_last_error.message;
  }

protected:
  AbstractNodeCompiler(BackendLanguage language, const NodeModel& model);
  [[nodiscard]] std::set<Node*> nodes() const;

  void generate_statements(std::set<QString>& used_node_types,
                           std::list<std::unique_ptr<Statement>>& statements) const;

public:
  const BackendLanguage language;
  [[nodiscard]] const NodeModel& model() const
  {
    return m_model;
  }
  QString code();
  QString error();
  virtual bool compile() = 0;
  static bool can_cast(Type from, Type to);

Q_SIGNALS:
  void compilation_succeeded(const QString& code);
  void compilation_failed(const QString& reason);

public:
  virtual void invalidate();
  [[nodiscard]] virtual std::set<Type> supported_types() const = 0;

protected:
  struct AssemblyError
  {
    AssemblyError(const QString& message = "");
    QString message;
  };

  AssemblyError m_last_error;
  QString m_code = "";
  bool m_is_dirty = true;
  bool check(const AssemblyError& error);

private:
  const NodeModel& m_model;

};

template<typename ConcreteCompiler> class NodeCompiler : public AbstractNodeCompiler
{
public:
  bool compile() override
  {
    m_is_dirty = false;
    m_code.clear();
    m_last_error = {};
    QStringList lines;
    std::set<QString> used_node_types;
    std::list<std::unique_ptr<Statement>> statements;
    generate_statements(used_node_types, statements);
    const auto& self = static_cast<const ConcreteCompiler&>(*this);

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

}  // namespace omm::nodes
