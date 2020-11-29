#pragma once

#include <QStringList>
#include <set>

namespace omm
{

class AbstractPort;
class Node;
class OutputPort;
class InputPort;

class Statement {
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
};

class NodeStatement : public Statement
{
public:
  NodeStatement(const Node& node);
  const Node* const node = nullptr;
  [[nodiscard]] bool is_connection() const override;
private:
  [[nodiscard]] std::set<const AbstractPort*> defines() const override;
  [[nodiscard]] std::set<const AbstractPort*> uses() const override;
};

class ConnectionStatement : public Statement
{
public:
  ConnectionStatement(const OutputPort& source, const InputPort& target);
  const OutputPort* const source = nullptr;
  const InputPort* const target = nullptr;
  [[nodiscard]] bool is_connection() const override;
private:
  [[nodiscard]] std::set<const AbstractPort*> defines() const override;
  [[nodiscard]] std::set<const AbstractPort*> uses() const override;
};

}  // namespace omm
