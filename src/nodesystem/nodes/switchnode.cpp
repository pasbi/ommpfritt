#include "nodesystem/nodes/switchnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/nodecompilerglsl.h"
#include "properties/integerproperty.h"
#include "properties/floatvectorproperty.h"

namespace
{

constexpr auto glsl_definition_template = R"(
%2 %1_0(int op, %2 a0, %2 a1, %2 a2, %2 a3, %2 a4, %2 a5, %2 a6, %2 a7, %2 a8, %2 a9) {
  switch (op) {
  case 0: return a0;
  case 1: return a1;
  case 2: return a2;
  case 3: return a3;
  case 4: return a4;
  case 5: return a5;
  case 6: return a6;
  case 7: return a7;
  case 8: return a8;
  case 9: return a9;
  default: return a0;
  }
})";

constexpr auto python_definition_template = R"(
def %1(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9):
  try:
    return [a0, a1, a2, a3, a4, a5, a6, a7, a8, a9][op]
  except:
    return %1()
)";

QString input_name(const std::size_t i)
{
  return QString{"a%1"}.arg(i);
}

constexpr auto n_options = 10;

auto glsl_definitions()
{
  QStringList overloads;
  constexpr auto types = std::array{omm::Type::FloatVector, omm::Type::Integer,
                                    omm::Type::Float, omm::Type::Color,
                                    omm::Type::IntegerVector};
  overloads.reserve(types.size());
  const auto template_definition = QString{glsl_definition_template}.arg(omm::nodes::SwitchNode::TYPE);
  for (const auto& type : types) {
    using omm::nodes::NodeCompilerGLSL;
    overloads.push_back(template_definition.arg(NodeCompilerGLSL::type_name(type)));
  }
  return overloads.join("\n");
}

}  // namespace

namespace omm::nodes
{

const Node::Detail SwitchNode::detail{
    .definitions = {
        {BackendLanguage::Python, QString{python_definition_template}.arg(SwitchNode::TYPE)},
        {BackendLanguage::GLSL, glsl_definitions()}
    },
  .menu_path = {QT_TRANSLATE_NOOP("NodeMenuPath", "Math")},
};

SwitchNode::SwitchNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Node");
  const auto& key_property = create_property<IntegerProperty>(KEY_KEY, 0)
      .set_label(tr("key"))
      .set_category(category);
  m_options.reserve(n_options);
  for (std::size_t i = 0; i < n_options; ++i) {
    m_options.push_back(&add_port<OrdinaryPort<PortType::Input>>(input_name(i)));
  }
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
  m_key_input_port = find_port<InputPort>(key_property);
}

Type SwitchNode::output_data_type(const OutputPort& port) const
{
  if (&port != m_output_port) {
    return Type::Invalid;
  }

  auto types = util::transform<std::set>(m_options, [](const InputPort* ip) {
    return ip->data_type();
  });
  types.erase(Type::Invalid);

  if (types.size() != 1) {
    return Type::Invalid;  // ambiguous type
  }
  return *types.begin();
}

QString SwitchNode::title() const
{
  return tr("Compose");
}

bool SwitchNode::accepts_input_data_type(const Type type, const InputPort& port, const bool with_cast) const
{
  if (&port == m_key_input_port) {
    if (with_cast) {
      return NodeCompilerGLSL::can_cast(type, Type::Integer);
    } else {
      return type == Type::Integer;
    }
  } else {
    return type != Type::Invalid;
  }
}

QString SwitchNode::type() const
{
  return TYPE;
}

InputPort* SwitchNode::find_surrogate_for(const InputPort& port) const
{
  if (!port.is_connected()) {
    std::deque<InputPort*> candidates(m_options.begin(), m_options.end());
    const auto it = std::find(candidates.begin(), candidates.end(), &port);
    std::rotate(candidates.begin(), it, candidates.end());
    std::reverse(candidates.begin(), candidates.end());
    for (auto* other_input_port : candidates) {
      if (other_input_port->is_connected()) {
        return other_input_port;
      }
    }
  }
  return nullptr;
}

Type SwitchNode::input_data_type(const InputPort& port) const
{
  if (&port == m_key_input_port) {
    return Type::Integer;
  } else if (const auto* surrogate = find_surrogate_for(port); surrogate == nullptr) {
    return Node::input_data_type(port);
  } else {
    return surrogate->data_type();
  }
}

QString SwitchNode::dangling_input_port_uuid(const InputPort& port) const
{
  if (const auto* surrogate = find_surrogate_for(port); surrogate == nullptr) {
    return Node::dangling_input_port_uuid(port);
  } else {
    return surrogate->uuid();
  }
}

}  // namespace omm::nodes
