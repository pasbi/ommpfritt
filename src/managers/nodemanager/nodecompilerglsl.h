#pragma once

#include "managers/nodemanager/nodecompiler.h"

namespace omm
{

class NodeCompilerGLSL : public NodeCompiler<NodeCompilerGLSL>
{
public:
  explicit NodeCompilerGLSL(const NodeModel& model);
  static constexpr auto language = AbstractNodeCompiler::Language::GLSL;
  static constexpr bool lazy = false;
  QString header() const;
  QString start_program() const;
  QString end_program() const;
  QString compile_node(const Node& node) const;
  QString compile_connection(const OutputPort& op, const InputPort& ip) const;
  QString define_node(const QString& node_type) const;
};

}  // namespace omm
