#pragma once

#include "nodesystem/nodecompiler.h"

namespace omm::nodes
{

class NodeCompilerPython : public NodeCompiler<NodeCompilerPython>
{
public:
  explicit NodeCompilerPython(const NodeModel& model);
  static constexpr auto LANGUAGE = BackendLanguage::Python;

  static QString generate_header(QStringList& lines);
  static QString start_program(QStringList&);
  static QString end_program(QStringList&);
  static QString compile_node(const Node& node, QStringList& lines);
  static QString compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines);
  QString define_node(const QString& node_type, QStringList& lines) const;
};

}  // namespace omm::nodes
