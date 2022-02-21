#include "nodesystem/nodecompilerpython.h"
#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/port.h"
#include "removeif.h"

namespace omm::nodes
{

NodeCompilerPython::NodeCompilerPython(const NodeModel& model) : NodeCompiler(model)
{
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::generate_header(QStringList& lines)
{
  lines.append(QString(R"(
def listarithm_decorator(func):
  def wrapper(*args, **kwargs):
      import numpy as np
      to_np = lambda x: np.array(x) if isinstance(x, list) else x
      fr_np = lambda x: list(x) if isinstance(x, np.ndarray) else x
      args = [ to_np(arg) for arg in args ]
      kwargs = { key: to_np(arg) for key, arg in kwargs }
      return fr_np(func(*args, **kwargs))
  return wrapper
)"));
  return {};
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::start_program(QStringList&)
{
  return {};
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::end_program(QStringList&)
{
  return {};
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::compile_node(const Node& node, QStringList& lines)
{
  const auto ops = util::remove_if(node.ports<OutputPort>(), [](OutputPort* op) {
    return op->flavor != PortFlavor::Ordinary;
  });

  auto ips = util::transform<std::vector>(node.ports<InputPort>());
  std::sort(ips.begin(), ips.end(), [](InputPort* ip1, InputPort* ip2) {
    return ip1->index < ip2->index;
  });

  const QStringList args = util::transform<QList>(ips, [](InputPort* ip) { return ip->uuid(); });

  if (!ops.empty()) {
    const QStringList uuids = util::transform<QList>(ops, [](const OutputPort* op) { return op->uuid(); });
    lines.append(QString("%1 = %2(%3)").arg(uuids.join(", "), node.type(), args.join(", ")));
  }
  return {};
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::compile_connection(const OutputPort& op,
                                               const InputPort& ip,
                                               QStringList& lines)
{
  lines.append(QString("%1 = %2").arg(ip.uuid(), op.uuid()));
  return {};
}

AbstractNodeCompiler::AssemblyError NodeCompilerPython::define_node(const QString& node_type, QStringList& lines) const
{
  lines.append(Node::detail(node_type).definitions.at(language));
  return {};
}

std::set<Type> NodeCompilerPython::supported_types() const
{
  return {
      Type::Bool, Type::Color, Type::Float, Type::FloatVector, Type::Spline,
      Type::Integer, Type::IntegerVector, Type::Option, Type::Reference, Type::String
  };
}
}  // namespace omm::nodes
