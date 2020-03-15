#pragma once

#include "nodesystem/node.h"

namespace omm
{

class InterpolateNode : public Node
{
  Q_OBJECT
public:
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "InterpolateNode");
  static constexpr auto T_PROPERTY_KEY = "t";
  explicit InterpolateNode(NodeModel& model);
  static const Detail detail;
  QString type() const { return TYPE; }
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString input_data_type(const InputPort& port) const override;
  QString output_data_type(const OutputPort& port) const override;

private:
  InputPort* m_a_input = nullptr;
  InputPort* m_b_input = nullptr;
  InputPort* m_t_input = nullptr;
  OutputPort* m_output = nullptr;
};

}  // namespace
