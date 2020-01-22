#include "managers/nodemanager/nodecompilerpython.h"
#include "common.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/port.h"

namespace omm
{

NodeCompilerPython::NodeCompilerPython(const NodeModel& model) : NodeCompiler(model) {  }

QString NodeCompilerPython::header() const
{
  return "";
}

QString NodeCompilerPython::start_program() const
{
  return "";
}

QString NodeCompilerPython::end_program() const
{
  return "";
}

QString NodeCompilerPython::compile_node(const Node& node) const
{
  auto ops = ::filter_if(node.ports<OutputPort>(), [](OutputPort* op) {
    return op->flavor == PortFlavor::Ordinary;
  });

  std::vector<InputPort*> ips = ::transform<InputPort*, std::vector>(node.ports<InputPort>(),
                                                                     ::identity);
  std::sort(ips.begin(), ips.end(), [](InputPort* ip1, InputPort* ip2) {
    return ip1->index < ip2->index;
  });

  const QStringList args = ::transform<QString, QList>(ips, [](InputPort* ip) {
    return ip->uuid();
  });

  if (ops.size() >= 1) {
    const QStringList uuids = ::transform<QString, QList>(ops, [](const OutputPort* op) {
      return op->uuid();
    });
    return QString("%1 = %2(%3)").arg(uuids.join(", ")).arg(node.type()).arg(args.join(", "));
  } else {
    return "";
  }
}

QString NodeCompilerPython::compile_connection(const OutputPort& op, const InputPort& ip) const
{
  return QString("%1 = %2").arg(ip.uuid()).arg(op.uuid());
}

QString NodeCompilerPython::define_node(const QString& node_type) const
{
  return Node::detail(node_type).definitions.at(language);
}

}  // namespace omm
