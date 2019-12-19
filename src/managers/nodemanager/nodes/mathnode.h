#include "managers/nodemanager/node.h"

namespace omm
{

class MathNode : public Node
{
public:
  explicit MathNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "MathNode");
  QString type() const override { return TYPE; }

  static constexpr auto A_PROPERTY_KEY = "a";
  static constexpr auto B_PROPERTY_KEY = "b";
  static constexpr auto OPERATION_PROPERTY_KEY = "op";

  QString definition(NodeCompiler::Language language) const override;
  QString name(NodeCompiler::Language language) const override;

protected:
  void on_property_value_changed(Property* property) override;

};

}  // namespace omm
