#pragma once

#include "nodesystem/nodecompiler.h"

namespace omm::nodes
{

class NodeCompilerGLSL : public NodeCompiler<NodeCompilerGLSL>
{
public:
  explicit NodeCompilerGLSL(const NodeModel& model);
  static constexpr auto LANGUAGE = BackendLanguage::GLSL;
  AssemblyError generate_header(QStringList& lines) const;
  static AssemblyError start_program(QStringList& lines);
  AssemblyError end_program(QStringList& lines) const;
  static AssemblyError compile_node(const Node& node, QStringList& lines);
  static AssemblyError compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines);
  AssemblyError define_node(const QString& node_type, QStringList& lines) const;
  std::set<AbstractPort*> uniform_ports() const;
  void invalidate() override;
  static constexpr std::size_t SPLINE_SIZE = 256;
  static QString type_name(Type type);
  [[nodiscard]] std::set<Type> supported_types() const override;

private:
  mutable std::set<AbstractPort*> m_uniform_ports;
};

}  // namespace omm::nodes
