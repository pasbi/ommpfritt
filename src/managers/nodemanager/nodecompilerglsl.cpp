#include "managers/nodemanager/nodecompilerglsl.h"
#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/nodecompiler.h"
#include "common.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"

namespace
{

static constexpr auto output_variable_name = "out_color";

QString translate_type(const QString& type)
{
  static const std::map<QString, QString> dict {
    { "Color", "vec4" },
    { "Reference", "uint" },
  };

  return dict.at(type);
}

}  // namespace

namespace omm
{

NodeCompilerGLSL::NodeCompilerGLSL(const NodeModel& model) : NodeCompiler(model) {  }

QString NodeCompilerGLSL::header() const
{
  QStringList lines { "#version 330", QString("out vec4 %1;").arg(output_variable_name) };

  for (Port<PortType::Output>* port : model().ports<OutputPort>()) {
    if (port->flavor == omm::PortFlavor::Property) {
      lines.push_back(QString("uniform %1 %2;")
                      .arg(translate_type(port->data_type()))
                      .arg(port->uuid()));
    }
  }
  return lines.join("\n");
}

QString NodeCompilerGLSL::start_program() const
{
  return "void main() {";
}

QString NodeCompilerGLSL::end_program() const
{
  const auto fragment_nodes = ::filter_if(model().nodes(), [](const Node* node) {
    return node->type() == FragmentNode::TYPE;
  });

  QStringList lines;
  if (fragment_nodes.size() != 1) {
    LWARNING << "expected exactly one fragment node but found " << fragment_nodes.size() << ".";
  } else {
    auto fn = static_cast<const FragmentNode*>(*fragment_nodes.begin());
    lines.push_back(QString("%1 = %2;").arg(output_variable_name).arg(fn->port_name()));
  }

  lines.push_back("}  // main()");
  return lines.join("\n");
}

QString NodeCompilerGLSL::compile_node(const Node& node) const
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

  if (ops.size() != 1) {
    LINFO <<  "Encountered GLSL node with " << ops.size() << " outputs.\n"
              "GLSL nodes must have exactly one output.";
    return "";
  } else {
    const OutputPort* port = *ops.begin();
    return QString("%1 %2 = %3(%4);")
        .arg(translate_type(port->data_type()))
        .arg(port->uuid())
        .arg(node.uuid())
        .arg(args.join(", "));
  }
}

QString NodeCompilerGLSL::compile_connection(const OutputPort& op, const InputPort& ip) const
{
  const QString data_type = op.data_type();
  assert(data_type != NodeCompilerTypes::INVALID_TYPE);
  return QString("%1 %2 = %3;")
      .arg(translate_type(op.data_type()))
      .arg(ip.uuid())
      .arg(op.uuid());
}

QString NodeCompilerGLSL::define_node(const QString& node_type) const
{
  return "";
}

}  // namespace omm
