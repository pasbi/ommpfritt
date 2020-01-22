#include "managers/nodemanager/node.h"

namespace omm
{

class ReferenceNode : public Node
{
  Q_OBJECT
public:
  explicit ReferenceNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ReferenceNode");
  QString type() const override { return TYPE; }

  static constexpr auto TYPE_PROPERTY_KEY = "data-type";
  static constexpr auto CONSTANT_VALUE_KEY = "constant";
  static constexpr auto REFERENCE_PROPERTY_KEY = "ref";

  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void populate_menu(QMenu& menu) override;
  QString title() const override;
  AbstractPort& add_forwarding_port(PortType port_type, const QString& key);
  std::unique_ptr<AbstractPort> remove_forwarding_port(PortType port_type, const QString& key);
  static const Detail detail;

protected:
  void on_property_value_changed(Property* property) override;

private:
  AbstractPropertyOwner* reference() const;
  std::map<PortType, std::map<QString, AbstractPort*>> m_forwarded_ports;

  std::unique_ptr<QAction>
  make_property_action(PortType port_type, const QString& key, const QString& label);
};

}  // namespace omm
