#include "nodesystem/nodecompilerpython.h"
#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/port.h"

namespace omm
{

NodeCompilerPython::NodeCompilerPython(const NodeModel& model) : NodeCompiler(model) {  }

QString NodeCompilerPython::generate_header(QStringList& lines) const
{
  Q_UNUSED(lines)
  return "";
}

QString NodeCompilerPython::start_program(QStringList& lines) const
{
  Q_UNUSED(lines)
  return "";
}

QString NodeCompilerPython::end_program(QStringList& lines) const
{
  Q_UNUSED(lines)
  return "";
}

QString NodeCompilerPython::compile_node(const Node& node, QStringList& lines) const
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
    lines.append(QString("%1 = %2(%3)").arg(uuids.join(", ")).arg(node.type()).arg(args.join(", ")));
  }
  return "";
}

QString NodeCompilerPython::compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines) const
{
  lines.append(QString("%1 = %2").arg(ip.uuid()).arg(op.uuid()));
  return "";
}

QString NodeCompilerPython::define_node(const QString& node_type, QStringList& lines) const
{
  lines.append(Node::detail(node_type).definitions.at(language));
  return "";
}

}  // namespace omm
