#include "nodesystem/nodecompilerglsl.h"
#include "common.h"
#include "nodesystem/node.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/vertexnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/port.h"
#include "renderers/offscreenrenderer.h"

namespace
{
constexpr auto output_variable_name = "out_color";

QString format_connection(const omm::AbstractPort& lhs, const omm::AbstractPort& rhs)
{
  return QString("%1 %2 = %3;")
      .arg(omm::NodeCompilerGLSL::translate_type(lhs.data_type()), lhs.uuid(), rhs.uuid());
}

omm::AbstractPort* get_sibling(const omm::AbstractPort* port)
{
  if (port->flavor != omm::PortFlavor::Property) {
    return nullptr;
  }
  static const auto get_property = [](const omm::AbstractPort* port) {
    return port->port_type == omm::PortType::Input
               ? dynamic_cast<const omm::PropertyInputPort*>(port)->property()
               : dynamic_cast<const omm::PropertyOutputPort*>(port)->property();
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
  static const std::map<QString, QString> dict{
      {NodeCompilerTypes::COLOR_TYPE, "vec4"},
      {NodeCompilerTypes::REFERENCE_TYPE, "uint"},
      {NodeCompilerTypes::BOOL_TYPE, "bool"},
      {NodeCompilerTypes::FLOAT_TYPE, "float"},
      {NodeCompilerTypes::INTEGER_TYPE, "int"},
      {NodeCompilerTypes::FLOATVECTOR_TYPE, "vec2"},
      {NodeCompilerTypes::INTEGERVECTOR_TYPE, "ivec2"},
      {NodeCompilerTypes::OPTION_TYPE, "int"},
      {NodeCompilerTypes::SPLINE_TYPE, "float[SPLINE_SIZE]"}};

  const auto it = dict.find(type);
  if (it == dict.end()) {
    return QString("INVALID[%1]").arg(type);
  } else {
    return dict.at(type);
  }
}

void NodeCompilerGLSL::invalidate()
{
  AbstractNodeCompiler::invalidate();
  compile();
}

NodeCompilerGLSL::NodeCompilerGLSL(const NodeModel& model) : NodeCompiler(model)
{
}

QString NodeCompilerGLSL::generate_header(QStringList& lines) const
{
  m_uniform_ports.clear();
  lines.append("#version 330");
  lines.append(QString("const int SPLINE_SIZE = %1;").arg(SPLINE_SIZE));
  using Kind = OffscreenRenderer::ShaderInput::Kind;
  static const std::map<Kind, QString> input_kind_identifier_map = {
      {Kind::Uniform, "uniform"},
      {Kind::Varying, "varying"},
  };
  for (const auto& shader_input : OffscreenRenderer::fragment_shader_inputs) {
    lines.append(QString("%1 %2 %3;")
                     .arg(input_kind_identifier_map.at(shader_input.kind),
                          translate_type(shader_input.type),
                          shader_input.name));
  }
  lines.append(QString("out vec4 %1;").arg(output_variable_name));

  for (OutputPort* port : model().ports<OutputPort>()) {
    // only property ports can be uniform
    if (port->flavor == omm::PortFlavor::Property) {
      AbstractPort* sibling = get_sibling(port);
      // if the sibling (same property) input port is connected, the non-uniform value is forwarded.
      // We don't need a uniform.
      if (sibling == nullptr || !sibling->is_connected()) {
        m_uniform_ports.insert(port);
      }
    }
  }
  for (InputPort* port : model().ports<InputPort>()) {
    // only property ports can be uniform
    if (port->flavor == omm::PortFlavor::Property) {
      auto* ip = dynamic_cast<PropertyInputPort*>(port);
      if (!ip->is_connected() && get_sibling(port) == nullptr) {
        m_uniform_ports.insert(port);
      }
    }
  }
  for (AbstractPort* port : m_uniform_ports) {
    lines.push_back(QString("uniform %1 %2;").arg(translate_type(port->data_type()), port->uuid()));
  }
  return "";
}

QString NodeCompilerGLSL::start_program(QStringList& lines)
{
  lines.append("void main() {");
  return "";
}

QString NodeCompilerGLSL::end_program(QStringList& lines) const
{
  if (const auto nodes = model().nodes(); !nodes.empty()) {
    const auto fragment_nodes
        = ::filter_if(nodes, [](const Node* node) { return node->type() == FragmentNode::TYPE; });

    if (fragment_nodes.size() != 1) {
      QString msg
          = QString("expected exactly one fragment node but found %1.").arg(fragment_nodes.size());
      LWARNING << msg;
      return msg;
    } else {
      const auto* fragment_node = dynamic_cast<const FragmentNode*>(*fragment_nodes.begin());
      const auto& port = fragment_node->input_port();
      if (port.is_connected()) {
        const QString alpha = QString("clamp(%1.a, 0.0, 1.0)").arg(port.uuid());
        const QString rgb = QString("clamp(%2.rgb, vec3(0.0), vec3(1.0))").arg(port.uuid());
        lines.push_back(QString("%1 = vec4(%2 * %3, %2);").arg(output_variable_name, alpha, rgb));
      }
    }
  }

  lines.append("}");

  return "";
}

QString NodeCompilerGLSL::compile_node(const Node& node, QStringList& lines)
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

  {
    auto ips = sort_ports(node.ports<InputPort>());
    const QStringList args = ::transform<QString, QList>(ips, [](InputPort* ip) {
      if (!ip->is_connected() && ip->flavor == PortFlavor::Property) {
        AbstractPort* op = get_sibling(ip);
        if (op != nullptr) {
          return op->uuid();
        }
      }
      return ip->uuid();
    });

    auto ordinary_output_ports = ::filter_if(node.ports<OutputPort>(), [](OutputPort* op) {
      return op->flavor == PortFlavor::Ordinary;
    });
    std::size_t i = 0;
    for (OutputPort* port : sort_ports(ordinary_output_ports)) {
      if (const Node& node = port->node; node.type() == VertexNode::TYPE) {
        const auto& vertex_node = dynamic_cast<const VertexNode&>(node);
        const auto& ports = vertex_node.shader_inputs();
        const auto it = std::find(ports.begin(), ports.end(), port);
        if (it != ports.end()) {
          lines.push_back(
              QString("%1 %2 = %3;")
                  .arg(translate_type(port->data_type()), port->uuid(), it->input_info.name));
        }
      } else {
        lines.push_back(QString("%1 %2 = %3_%4(%5);")
                            .arg(translate_type(port->data_type()), port->uuid(), node.type())
                            .arg(i)
                            .arg(args.join(", ")));
      }
      i += 1;
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
        if (sibling_input_port != nullptr && sibling_input_port->is_connected()) {
          assert(sibling_input_port->port_type == PortType::Input);
          lines.push_back(format_connection(*op, *sibling_input_port));
        }
      }
    }
  }
  return "";
}

QString
NodeCompilerGLSL::compile_connection(const OutputPort& op, const InputPort& ip, QStringList& lines)
{
  lines.append(format_connection(ip, op));
  return "";
}

QString NodeCompilerGLSL::define_node(const QString& node_type, QStringList& lines) const
{
  lines.append(Node::detail(node_type).definitions.at(language));
  return "";
}

}  // namespace omm
