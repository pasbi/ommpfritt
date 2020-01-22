#pragma once

#include "managers/nodemanager/node.h"

namespace omm
{

template<PortType> class OrdinaryPort;

class ComposeNode : public Node
{
  Q_OBJECT
public:
  explicit ComposeNode(NodeModel& model);
  static constexpr auto INPUT_X_PROPERTY_KEY = "x";
  static constexpr auto INPUT_Y_PROPERTY_KEY = "y";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ComposeNode");

  QString output_data_type(const OutputPort& port) const override;
  QString title() const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString type() const override { return TYPE; }
  static const Detail detail;

private:
  OutputPort* m_output_port;
};

}  // namespace omm
