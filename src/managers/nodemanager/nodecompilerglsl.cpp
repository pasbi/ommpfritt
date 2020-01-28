#include "managers/nodemanager/nodecompilerglsl.h"
#include "managers/nodemanager/nodes/vertexnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/nodecompiler.h"
#include "common.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"
#include "renderers/offscreenrenderer.h"

namespace
{

static constexpr auto output_variable_name = "out_color";

QString format_connection(const omm::AbstractPort& lhs, const omm::AbstractPort& rhs)
{
  return QString("%1 %2 = %3;")
      .arg(omm::NodeCompilerGLSL::translate_type(lhs.data_type()))
      .arg(lhs.uuid())
      .arg(rhs.uuid());
}

omm::AbstractPort* get_sibling(const omm::AbstractPort* port)
{
  if (port->flavor != omm::PortFlavor::Property) {
    return nullptr;
  }
  static const auto get_property = [](const omm::AbstractPort* port) {
    return port->port_type == omm::PortType::Input
      ? static_cast<const omm::PropertyInputPort*>(port)->property()
      : static_cast<const omm::PropertyOutputPort*>(port)->property();
  };
  const omm::Property* property = get_property(port);
  for (omm::AbstractPort* candidate : port->node.ports()) {
    if (port->flavor == candidate->flavor && port->port_type != candidate->port_type) {
      if (property == get_property(candidate)) {
        return candidate;
      }
    }
  }
  return nullptr;
}

}  // namespace

namespace omm
{

QString NodeCompilerGLSL::translate_type(const QString& type)
{
  static const std::map<QString, QString> dict {
    { "Color", "vec4" },
    { "Reference", "uint" },
    { "Bool", "bool" },
    { "Float", "float" },
    { "Integer", "int" },
    { "FloatVector", "vec2" },
    { "IntegerVector", "ivec2" },
    { "Options", "int" }
  };

  return dict.at(type);
}


NodeCompilerGLSL::NodeCompilerGLSL(const NodeModel& model) : NodeCompiler(model) {  }

QString NodeCompilerGLSL::header() const
{
  m_uniform_ports.clear();
  QStringList lines {
    "#version 330",
    QString("varying vec2 %1;").arg(OffscreenRenderer::vertex_position_name),
    QString("out vec4 %1;").arg(output_variable_name)
  };

  for (Port<PortType::Output>* port : model().ports<OutputPort>()) {
    // only property ports can be uniform
    if (port->flavor == omm::PortFlavor::Property) {
      // inputs cannot be uniform
      if (port->port_type == PortType::Output) {
        AbstractPort* sibling = get_sibling(port);
        // if the sibling (same property) input port is connected, the non-uniform value is forwarded.
        // We don't need a uniform.
        if (sibling == nullptr || !sibling->is_connected()) {
          m_uniform_ports.insert(static_cast<InputPort*>(sibling));
          lines.push_back(QString("uniform %1 %2;")
                          .arg(translate_type(sibling->data_type()))
                          .arg(sibling->uuid()));
        }
      }
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
    const auto* fragment_node = static_cast<const FragmentNode*>(*fragment_nodes.begin());
    const auto& port = fragment_node->input_port();
    if (port.is_connected()) {
      lines.push_back(QString("%1 = %2;").arg(output_variable_name).arg(port.uuid()));
    }
  }

  lines.push_back("}  // main()");
  return lines.join("\n");
}

QString NodeCompilerGLSL::compile_node(const Node& node) const
{
  static const auto sort_ports = [](const auto& ports) {
    using PortT = std::decay_t<typename std::decay_t<decltype(ports)>::value_type>;
    auto vec = ::transform<PortT, std::vector>(ports, ::identity);
    std::sort(vec.begin(), vec.end(), [](const auto* p1, const auto* p2) {
      assert(p1->port_type == p2->port_type && &p1->node == &p2->node);
      return p1->index < p2->index;
    });
    return vec;
  };

  QStringList lines;
  {
    auto ips = sort_ports(node.ports<InputPort>());
    const QStringList args = ::transform<QString, QList>(ips, [](InputPort* ip) {
      return ip->uuid();
    });

    auto ordinary_output_ports = ::filter_if(node.ports<OutputPort>(), [](OutputPort* op) {
      return op->flavor == PortFlavor::Ordinary;
    });
    for (OutputPort* port : sort_ports(ordinary_output_ports)) {
      if (const Node& node = port->node; node.type() == VertexNode::TYPE) {
        const auto& vertex_node = static_cast<const VertexNode&>(node);
        if (port == &vertex_node.position_port()) {
          lines.push_back(QString("%1 %2 = %3;")
                          .arg(translate_type(port->data_type()))
                          .arg(port->uuid())
                          .arg(OffscreenRenderer::vertex_position_name));
        }
      } else {
        lines.push_back(QString("%1 %2 = %3_%4(%5);")
            .arg(translate_type(port->data_type()))
            .arg(port->uuid())
            .arg(node.type())
            .arg(lines.size())
            .arg(args.join(", ")));
      }
    }
  }

  {
    auto leftover_property_output_ports = ::filter_if(node.ports<OutputPort>(), [](OutputPort* op) {
      return op->flavor == PortFlavor::Property && get_sibling(op) != nullptr;
    });
    for (OutputPort* op : node.ports<OutputPort>()) {
      // only handle some output ports.
      // The remainding ones represent uniform variables. see NodeCompilerGLSL::header
      if (op->flavor == PortFlavor::Property) {
        AbstractPort* sibling_input_port = get_sibling(op);
        if (sibling_input_port != nullptr) {
          assert(sibling_input_port->port_type == PortType::Input);
          lines.push_back(format_connection(*op, *sibling_input_port));
        }
      }
    }
  }


  return lines.join("\n");
}

QString NodeCompilerGLSL::compile_connection(const OutputPort& op, const InputPort& ip) const
{
  const QString data_type = op.data_type();
  assert(data_type != NodeCompilerTypes::INVALID_TYPE);

  return format_connection(ip, op);
}

QString NodeCompilerGLSL::define_node(const QString& node_type) const
{
  return Node::detail(node_type).definitions.at(language);
}

}  // namespace omm
