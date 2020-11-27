#pragma once

#include "nodesystem/nodecompiler.h"

namespace omm
{
class NodeCompilerGLSL : public NodeCompiler<NodeCompilerGLSL>
{
public:
  explicit NodeCompilerGLSL(const NodeModel& model);
  static constexpr auto LANGUAGE = AbstractNodeCompiler::Language::GLSL;
  QString generate_header(QStringList& lines) const;
  static QString start_program(QStringList& lines);
  QString end_program(QStringList& lines) const;
  static QString compile_node(const Node& node, QStringList& lines);
  static QString compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines);
  QString define_node(const QString& node_type, QStringList& lines) const;
  std::set<AbstractPort*> uniform_ports() const
  {
    return m_uniform_ports;
  }
  static QString translate_type(const QString& type);
  void invalidate() override;
  static constexpr std::size_t SPLINE_SIZE = 256;

private:
  mutable std::set<AbstractPort*> m_uniform_ports;
};

}  // namespace omm
