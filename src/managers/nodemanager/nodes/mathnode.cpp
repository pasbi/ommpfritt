#include "managers/nodemanager/nodes/mathnode.h"
#include "managers/nodemanager/ordinaryport.h"
#include "properties/floatproperty.h"
#include "managers/nodemanager/propertyport.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace
{

using namespace omm::NodeCompilerTypes;
enum class Operation { Addition, Difference, Multiplication, Division, Power };

auto type_matrix()
{
  std::map<std::tuple<Operation, QString, QString>, QString> matrix {
    { { Operation::Addition, FLOAT_TYPE,         FLOAT_TYPE },         FLOAT_TYPE },
    { { Operation::Addition, FLOAT_TYPE,         INTEGER_TYPE },       FLOAT_TYPE },
    { { Operation::Addition, FLOAT_TYPE,         BOOL_TYPE },          FLOAT_TYPE },
    { { Operation::Addition, INTEGER_TYPE,       INTEGER_TYPE },       INTEGER_TYPE },
    { { Operation::Addition, INTEGER_TYPE,       BOOL_TYPE },          INTEGER_TYPE },
    { { Operation::Addition, BOOL_TYPE,          BOOL_TYPE },          BOOL_TYPE },
    { { Operation::Addition, FLOATVECTOR_TYPE,   FLOATVECTOR_TYPE },   FLOATVECTOR_TYPE },
    { { Operation::Addition, INTEGERVECTOR_TYPE, FLOATVECTOR_TYPE },   FLOATVECTOR_TYPE },
    { { Operation::Addition, INTEGERVECTOR_TYPE, INTEGERVECTOR_TYPE }, INTEGERVECTOR_TYPE },

    { { Operation::Multiplication, FLOAT_TYPE,   FLOAT_TYPE },         FLOAT_TYPE },
    { { Operation::Multiplication, FLOAT_TYPE,   INTEGER_TYPE },       FLOAT_TYPE },
    { { Operation::Multiplication, FLOAT_TYPE,   BOOL_TYPE },          FLOAT_TYPE },
    { { Operation::Multiplication, FLOAT_TYPE,   FLOATVECTOR_TYPE },   FLOATVECTOR_TYPE },
    { { Operation::Multiplication, FLOAT_TYPE,   INTEGERVECTOR_TYPE }, FLOATVECTOR_TYPE },
    { { Operation::Multiplication, INTEGER_TYPE, INTEGER_TYPE },       INTEGER_TYPE },
    { { Operation::Multiplication, INTEGER_TYPE, BOOL_TYPE },          INTEGER_TYPE },
    { { Operation::Multiplication, INTEGER_TYPE, FLOATVECTOR_TYPE },   FLOATVECTOR_TYPE },
    { { Operation::Multiplication, INTEGER_TYPE, INTEGERVECTOR_TYPE }, INTEGERVECTOR_TYPE },

    { { Operation::Power, FLOAT_TYPE,   FLOAT_TYPE },   FLOAT_TYPE },
    { { Operation::Power, FLOAT_TYPE,   INTEGER_TYPE }, FLOAT_TYPE },
    { { Operation::Power, FLOAT_TYPE,   BOOL_TYPE },    FLOAT_TYPE },
    { { Operation::Power, INTEGER_TYPE, INTEGER_TYPE }, INTEGER_TYPE },
    { { Operation::Power, INTEGER_TYPE, BOOL_TYPE },    INTEGER_TYPE },
    { { Operation::Power, BOOL_TYPE,    BOOL_TYPE },    BOOL_TYPE },
  };

  auto copy = matrix;
  for (const auto& [in, out] : matrix) {
    if (std::get<0>(in) == Operation::Addition) {
      // Difference has the same type inference rules as Addition.
      copy.insert({ { Operation::Difference, std::get<1>(in), std::get<2>(in) }, out });
    }

    if (std::get<0>(in) == Operation::Multiplication) {
      // Division has reversed inference rules as Multiplication:
      // if A*B yields type C then B/A yields type C.
      copy.insert({ { Operation::Division, std::get<2>(in), std::get<1>(in) }, out });
    }
  }

  matrix = copy;
  for (const auto& [in, out] : matrix) {
    if (   std::get<0>(in) == Operation::Addition
        || std::get<0>(in) == Operation::Difference
        || std::get<0>(in) == Operation::Multiplication
        || std::get<0>(in) == Operation::Power)
    {
      // Type inference rules of Addition, Difference, Multiplication and Power are symmetric.
      copy.insert({ { std::get<0>(in), std::get<2>(in), std::get<1>(in) }, out });
    }
  }

  return copy;
};

}  // namespace

namespace omm
{

MathNode::MathNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  create_property<OptionsProperty>(OPERATION_PROPERTY_KEY, 0.0)
      .set_options({ tr("+"), tr("-"), tr("*"), tr("/"), tr("pow") })
      .set_label(QObject::tr("Operation")).set_category(category);
  create_property<FloatProperty>(A_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("a")).set_category(category);
  create_property<FloatProperty>(B_PROPERTY_KEY, 0.0)
      .set_label(QObject::tr("b")).set_category(category);
  m_result_port = &add_port<OrdinaryPort<PortType::Output>>(tr("result"));
}

QString MathNode::definition() const
{
  return QString(R"(
def %1(op, a, b):
    def do_op(op, a, b):
        if op == 0:
            return a + b
        elif op == 1:
            return a - b
        elif op == 2:
            return a * b
        elif op == 3:
            return a / b
        elif op == 4:
            return a ** b
        else:
            return 0.0;
    return float(do_op(op, a, b))
)").arg(uuid());
}

QString MathNode::uuid() const
{
  return "math";
}

std::unique_ptr<Node> MathNode::clone() const
{
  return std::make_unique<MathNode>(*this);
}

QString MathNode::output_data_type(const OutputPort& port) const
{
  if (&port == m_result_port) {
    const QString type_a = find_port<InputPort>(*property(A_PROPERTY_KEY))->data_type();
    const QString type_b = find_port<InputPort>(*property(B_PROPERTY_KEY))->data_type();
    if (language() == NodeCompiler::Language::GLSL) {
      if (type_a == type_b) {
        return type_a;
      }
    } else if (language() == NodeCompiler::Language::Python) {
      const auto op = property(OPERATION_PROPERTY_KEY)->value<Operation>();
      const auto type_matrix = ::type_matrix();
      if (const auto it = type_matrix.find({op, type_a, type_b}); it != type_matrix.end()) {
        return it->second;
      }
    }
  }
  return NodeCompilerTypes::INVALID_TYPE;
}

bool MathNode::accepts_input_data_type(const QString& type, const InputPort& port) const
{
  assert(&port.node == this);
  assert(port.flavor == PortFlavor::Property);
  const auto* property = static_cast<const PropertyPort<PortType::Input>&>(port).property();
  if (property == this->property(OPERATION_PROPERTY_KEY)) {
    return Node::accepts_input_data_type(type, port);
  } else {
    // Math Node input shall accept anything
    return true;
  }
}

QString MathNode::title() const
{
  auto&& opp = static_cast<const OptionsProperty&>(*property(OPERATION_PROPERTY_KEY));
  const std::size_t i = opp.value();
  return Node::title() + tr(" [%1]").arg( opp.options().at(i));
}

void MathNode::on_property_value_changed(Property* property)
{
}

}  // namespace
