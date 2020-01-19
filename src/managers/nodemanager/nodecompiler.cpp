#include "managers/nodemanager/nodecompiler.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/nodemodel.h"
#include <sstream>

namespace omm
{

NodeCompiler::NodeCompiler(Language language) : m_language(language)
{
}

NodeCompiler::NodeCompiler(Language language, const NodeModel& model)
  : NodeCompiler(language)
{
  compile(model);
}

void NodeCompiler::compile(const NodeModel& model)
{
  m_error_message.clear();

  static const auto is_terminal = [](Node* node) {
    const auto output_ports = node->ports<OutputPort>();
    return std::all_of(output_ports.begin(), output_ports.end(), [](OutputPort* op) {
      return !op->is_connected();
    });
  };
  std::list<Node*> todo = ::transform<Node*, std::list>(::filter_if(model.nodes(), is_terminal));
  std::set<Node*> done;

  std::map<QString, QString> node_definitions;

  QStringList code;
  while (!todo.empty()) {
    Node* node = todo.front();
    todo.pop_front();
    code.push_front(compile_node(*node));
    for (InputPort* ip : node->ports<InputPort>()) {
      if (OutputPort* op = ip->connected_output(); op != nullptr) {
        code.push_front(compile_connection(*op, *ip));
        if (!::contains(done, &op->node)) {
          todo.push_back(&op->node);
        }
      }
    }
    {
      const auto name = node->uuid();
      auto it = node_definitions.find(name);
      if (it == node_definitions.end()) {
        node_definitions[name] = node->definition();
      } else {
        assert(it->second == node->definition());
      }
    }
    done.insert(node);
  }

  for (auto [name, definition] : node_definitions) {
    m_compilation += definition + "\n";
  }

  if (model.language() == Language::GLSL) {
    m_compilation += "void main() {\n";
    for (QString& string : code) {
      string = "  " + string;
    }
    m_compilation += code.join("\n");
    m_compilation += "\n}\n";
  } else {
    m_compilation += code.join("\n");
  }

  if (m_on_compilation_success_cb) {
    m_on_compilation_success_cb(m_compilation);
  }
}

void NodeCompiler::set_on_compilation_success_cb(const std::function<void (const QString&)>& cb)
{
  m_on_compilation_success_cb = cb;
}

QString NodeCompiler::compile_node(const Node& node)
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

  if (ops.size() > 1) {
    const QStringList uuids = ::transform<QString, QList>(ops, [](const OutputPort* op) {
      return op->uuid();
    });
    switch (m_language) {
    case Language::Python:
      return QString("%1 = %2(%3)").arg(uuids.join(", ")).arg(node.uuid()).arg(args.join(", "));
    case Language::GLSL:
      {
        if (ops.size() > 1) {
          LINFO <<  "Encountered GLSL node with " << ops.size() << " outputs.\n"
                    "GLSL nodes must not have more than one output.";
        }
        const OutputPort* port = *ops.begin();
        return QString("%1 %2 = %3(%4);")
            .arg(translate_type(port->data_type()))
            .arg(port->uuid())
            .arg(node.uuid())
            .arg(args.join(", "));
      }
    default:
      Q_UNREACHABLE();
    }
  } else {
    return "";
  }
}

QString NodeCompiler::compile_connection(const OutputPort& op, const InputPort& ip)
{
  switch (m_language) {
  case Language::Python:
    return QString("%1 = %2").arg(ip.uuid()).arg(op.uuid());
  case Language::GLSL:
  {
    const QString data_type = op.data_type();
    assert(data_type != NodeCompilerTypes::INVALID_TYPE);
    Property* prop = static_cast<const PropertyPort<PortType::Output>&>(op).property();
    LINFO << "data type: " << data_type << "  property: " << (!prop ? "null" : prop->type());
  }
    return QString("%1 %2 = %3;")
        .arg(translate_type(op.data_type()))
        .arg(ip.uuid())
        .arg(op.uuid());
  default:
    Q_UNREACHABLE();
    return "";
  }
}

QString NodeCompiler::translate_type(const QString& type) const
{
  assert(m_language == NodeCompiler::Language::GLSL);  // python typing is implicit
  static const std::map<QString, QString> dict {
    { "Color", "vec4" },
    { "Reference", "uint" },
  };

  return dict.at(type);
}

bool NodeCompilerTypes::is_integral(const QString& type)
{
  return type == BOOL_TYPE || type == INTEGER_TYPE || type == OPTIONS_TYPE;
}

bool NodeCompilerTypes::is_numeric(const QString& type)
{
  return is_integral(type) || type == FLOAT_TYPE;
}

bool NodeCompilerTypes::is_vector(const QString& type)
{
  return type == INTEGERVECTOR_TYPE || type == FLOATVECTOR_TYPE;
}

}  // namespace omm
