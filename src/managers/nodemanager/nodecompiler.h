#pragma once

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
}

class NodeCompiler
{
public:
  enum class Language { Python, GLSL };
  explicit NodeCompiler(Language language);
  NodeCompiler(Language language, const NodeModel& model);
  void compile(const NodeModel& model);

  QString compilation() const { return m_compilation; }
  QString error_message() const { return m_error_message; }
  bool has_error() const { return !m_error_message.isEmpty(); }
  QString uuid(const AbstractPort& port) const;
  QString uuid(const Node& node) const;

private:
  const Language m_language;
  QString m_error_message;
  QString m_compilation;
  void book(const Node& node);
  QString compile_node(const Node& node);
  QString compile_connection(const OutputPort& op, const InputPort& ip);
};

}  // namespace omm
