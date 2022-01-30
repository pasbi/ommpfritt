#pragma once

#include <QStringList>
#include <set>

namespace omm::nodes
{

class AbstractPort;
class Node;
class OutputPort;
class InputPort;

class Statement
{
public:
  Statement() = default;
  virtual ~Statement() = default;
  Statement(Statement&&) = delete;
  Statement(const Statement&) = delete;
  Statement& operator=(Statement&&) = delete;
  Statement& operator=(const Statement&) = delete;

  bool operator<(const Statement& other) const;
  [[nodiscard]] virtual bool is_connection() const = 0;
  [[nodiscard]] virtual std::set<const AbstractPort*> defines() const = 0;
  [[nodiscard]] virtual std::set<const AbstractPort*> uses() const = 0;
  [[nodiscard]] QString to_string() const;
};

class NodeStatement : public Statement
{
public:
  NodeStatement(const Node& node);
  const Node& node;
  [[nodiscard]] bool is_connection() const override;

private:
  [[nodiscard]] std::set<const AbstractPort*> defines() const override;
  [[nodiscard]] std::set<const AbstractPort*> uses() const override;
};

class ConnectionStatement : public Statement
{
public:
  ConnectionStatement(const OutputPort& source, const InputPort& target);
  const OutputPort& source;
  const InputPort& target;
  [[nodiscard]] bool is_connection() const override;

private:
  [[nodiscard]] std::set<const AbstractPort*> defines() const override;
  [[nodiscard]] std::set<const AbstractPort*> uses() const override;
};

}  // namespace omm::nodes
