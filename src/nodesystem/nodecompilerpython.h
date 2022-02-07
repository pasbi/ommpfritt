#pragma once

#include "nodesystem/nodecompiler.h"

namespace omm::nodes
{

class NodeCompilerPython : public NodeCompiler<NodeCompilerPython>
{
public:
  explicit NodeCompilerPython(const NodeModel& model);
  static constexpr auto LANGUAGE = BackendLanguage::Python;

  static AssemblyError generate_header(QStringList& lines);
  static AssemblyError start_program(QStringList&);
  static AssemblyError end_program(QStringList&);
  static AssemblyError compile_node(const Node& node, QStringList& lines);
  static AssemblyError compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines);
  AssemblyError define_node(const QString& node_type, QStringList& lines) const;
  [[nodiscard]] std::set<Type> supported_types() const override;
};

}  // namespace omm::nodes
