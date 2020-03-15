#include "nodesystem/nodes/interpolatenode.h"
#include "properties/floatproperty.h"
#include "nodesystem/ordinaryport.h"

namespace omm
{

const Node::Detail InterpolateNode::detail {
  {
    {
      AbstractNodeCompiler::Language::Python,
      QString(R"(
@listarithm_decorator
def %1(t, a, b):
  return (1-t) * a + t * b
)").arg(InterpolateNode::TYPE)
    },
    {
      AbstractNodeCompiler::Language::GLSL,
      ""
    },
  },
  {
    QT_TRANSLATE_NOOP("NodeMenuPath", "Interpolation"),
  },

};

InterpolateNode::InterpolateNode(NodeModel& model) : Node(model)
{
  const QString category = tr("Interpolation");
  const auto& t_property = create_property<FloatProperty>(T_PROPERTY_KEY, 0.0)
      .set_range(0.0, 1.0).set_step(0.01)
      .set_label(QObject::tr("t")).set_category(category);
  m_a_input = &add_port<OrdinaryPort<PortType::Input>>(tr("a"));
  m_b_input = &add_port<OrdinaryPort<PortType::Input>>(tr("b"));
  m_output = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
  m_t_input = find_port<InputPort>(t_property);
}

bool InterpolateNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_t_input) {
    return type == FLOAT_TYPE;
  } else {
    switch (language()) {
    case AbstractNodeCompiler::Language::GLSL:
      return false;
    case AbstractNodeCompiler::Language::Python:
      return true;
    default:
      Q_UNREACHABLE();
      return false;
    }
  }
}

QString InterpolateNode::input_data_type(const InputPort& port) const
{
  Q_UNUSED(port)
  return fst_con_ptype({ m_a_input, m_b_input }, QString(NodeCompilerTypes::FLOAT_TYPE));
}

QString InterpolateNode::output_data_type(const OutputPort& port) const
{
  using namespace NodeCompilerTypes;
  if (&port == m_output) {
    const QString type_a = m_a_input->data_type();
    const QString type_b = m_b_input->data_type();
    switch (language()) {
    case AbstractNodeCompiler::Language::GLSL:
      return type_a;
    case AbstractNodeCompiler::Language::Python:
      if (is_numeric(type_a) && is_numeric(type_b)) {
        return FLOAT_TYPE;
      } else if ((type_a == INTEGERVECTOR_TYPE && type_b == INTEGERVECTOR_TYPE)) {
        return FLOATVECTOR_TYPE;
      } else if ((type_a == FLOATVECTOR_TYPE && type_b == FLOATVECTOR_TYPE)) {
        return FLOATVECTOR_TYPE;
      } else {
        return INVALID_TYPE;
      }
    default:
      Q_UNREACHABLE();
    }
  }
  return INVALID_TYPE;
}

}  // namespace omm
