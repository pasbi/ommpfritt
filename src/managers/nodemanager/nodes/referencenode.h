#include "managers/nodemanager/node.h"

namespace omm
{

class ReferenceNode : public Node
{
public:
  explicit ReferenceNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ReferenceNode");
  QString type() const override { return TYPE; }

  static constexpr auto TYPE_PROPERTY_KEY = "data-type";
  static constexpr auto CONSTANT_VALUE_KEY = "constant";
  static constexpr auto REFERENCE_PROPERTY_KEY = "ref";

  std::unique_ptr<Menu> make_menu() override;

protected:
  void on_property_value_changed(Property* property) override;

private:
  AbstractPropertyOwner* reference() const;
  std::map<PortType, std::map<QString, AbstractPort*>> m_forwarded_ports;
  template<PortType port_type>
  std::unique_ptr<QAction> make_property_action(const QString& label, const QString& key);
};

}  // namespace omm
