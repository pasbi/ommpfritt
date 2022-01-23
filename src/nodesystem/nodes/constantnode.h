#pragma once

#include "nodesystem/node.h"

namespace omm::nodes
{

class ConstantNode : public Node
{
  Q_OBJECT
public:
  explicit ConstantNode(NodeModel& model);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ConstantNode");
  QString type() const override;

  static constexpr auto CONSTANT_VALUE_KEY = "constant";
  void populate_menu(QMenu& menu) override;
  static const Detail detail;
};

}  // namespace omm::nodes
