#include "nodesystem/nodes/switchnode.h"
#include "nodesystem/ordinaryport.h"
#include "nodesystem/nodecompilerglsl.h"
#include "properties/integerproperty.h"
#include "properties/floatvectorproperty.h"

namespace omm::nodes
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
  }
})";

constexpr auto python_definition_template = R"(
@listarithm_decorator
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
  constexpr auto types = std::array{types::FLOATVECTOR_TYPE, types::INTEGER_TYPE,
                                    types::FLOAT_TYPE, types::COLOR_TYPE,
                                    types::INTEGERVECTOR_TYPE};
  overloads.reserve(types.size());
  const auto template_definition = QString{glsl_definition_template}.arg(omm::nodes::SwitchNode::TYPE);
  for (const auto& type : types) {
    using omm::nodes::NodeCompilerGLSL;
    overloads.push_back(template_definition.arg(NodeCompilerGLSL::translate_type(type)));
  }
  return overloads.join("\n");
}

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
  create_property<IntegerProperty>(KEY_KEY, 0)
      .set_label(tr("key"))
      .set_category(category);
  m_options.reserve(n_options);
  for (std::size_t i = 0; i < n_options; ++i) {
    m_options.push_back(&add_port<OrdinaryPort<PortType::Input>>(input_name(i)));
  }
  m_output_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString SwitchNode::output_data_type(const OutputPort& port) const
{
  if (&port != m_output_port) {
    return types::INVALID_TYPE;
  }

  auto types = ::transform<QString, std::set>(m_options, [](const InputPort* ip) {
    return ip->data_type();
  });
  types.erase(types::INVALID_TYPE);

  if (types.size() != 1) {
    return types::INVALID_TYPE;  // ambiguous type
  }
  return *types.begin();
}

QString SwitchNode::title() const
{
  return tr("Compose");
}

bool SwitchNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  Q_UNUSED(port)
  return type != types::INVALID_TYPE;
}

QString SwitchNode::type() const
{
  return TYPE;
}

}  // namespace omm::nodes
