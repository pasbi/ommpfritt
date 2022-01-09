#include "nodesystem/nodecompilerpython.h"
#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/port.h"

namespace omm::nodes
{

NodeCompilerPython::NodeCompilerPython(const NodeModel& model) : NodeCompiler(model)
{
}

QString NodeCompilerPython::generate_header(QStringList& lines)
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
  return "";
}

QString NodeCompilerPython::start_program(QStringList&)
{
  return "";
}

QString NodeCompilerPython::end_program(QStringList&)
{
  return "";
}

QString NodeCompilerPython::compile_node(const Node& node, QStringList& lines)
{
  auto ops = ::filter_if(node.ports<OutputPort>(),
                         [](OutputPort* op) { return op->flavor == PortFlavor::Ordinary; });

  std::vector<InputPort*> ips
      = ::transform<InputPort*, std::vector>(node.ports<InputPort>(), ::identity);
  std::sort(ips.begin(), ips.end(), [](InputPort* ip1, InputPort* ip2) {
    return ip1->index < ip2->index;
  });

  const QStringList args
      = ::transform<QString, QList>(ips, [](InputPort* ip) { return ip->uuid(); });

  if (!ops.empty()) {
    const QStringList uuids
        = ::transform<QString, QList>(ops, [](const OutputPort* op) { return op->uuid(); });
    lines.append(QString("%1 = %2(%3)").arg(uuids.join(", "), node.type(), args.join(", ")));
  }
  return "";
}

QString NodeCompilerPython::compile_connection(const OutputPort& op,
                                               const InputPort& ip,
                                               QStringList& lines)
{
  lines.append(QString("%1 = %2").arg(ip.uuid(), op.uuid()));
  return "";
}

QString NodeCompilerPython::define_node(const QString& node_type, QStringList& lines) const
{
  lines.append(Node::detail(node_type).definitions.at(language));
  return "";
}

}  // namespace omm::nodes
