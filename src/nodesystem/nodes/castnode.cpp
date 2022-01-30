#include "nodesystem/nodes/castnode.h"
#include "properties/optionproperty.h"
#include "nodesystem/nodecompilerglsl.h"
#include <map>


namespace omm::nodes
{

bool CastNodeBase::is_convertible(const QString& from, const QString& to)
{
  static const std::set<QString> scalar {types::INTEGER_TYPE, types::OPTION_TYPE, types::FLOAT_TYPE, types::BOOL_TYPE};
  static const std::set<QString> two_dim {types::FLOATVECTOR_TYPE, types::INTEGERVECTOR_TYPE};

  return (scalar.contains(from) && scalar.contains(to)) || (two_dim.contains(from) && two_dim.contains(to));
}

QString CastNodeBase::translate_type(const QString& type)
{
  return NodeCompilerGLSL::translate_type(type);
}

}  // namespace omm::nodes
