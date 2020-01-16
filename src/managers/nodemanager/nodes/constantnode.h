#include "managers/nodemanager/node.h"

namespace omm
{

class ConstantNode : public Node
{
  Q_OBJECT
public:
  explicit ConstantNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ConstantNode");
  QString type() const override { return TYPE; }

  static constexpr auto CONSTANT_VALUE_KEY = "constant";
  void populate_menu(QMenu& menu) override;
  static const Detail detail;

protected:
  void on_property_value_changed(Property* property) override;

};

}  // namespace omm
