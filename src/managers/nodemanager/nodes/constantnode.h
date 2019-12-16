#include "managers/nodemanager/node.h"

namespace omm
{

class ConstantNode : public Node
{
public:
  explicit ConstantNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ConstantNode");
  QString type() const override { return TYPE; }

  static constexpr auto TYPE_PROPERTY_KEY = "data-type";
  static constexpr auto CONSTANT_VALUE_KEY = "constant";

protected:
  void on_property_value_changed(Property* property) override;

};

}  // namespace omm
