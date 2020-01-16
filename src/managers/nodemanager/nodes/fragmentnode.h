#include "managers/nodemanager/node.h"

namespace omm
{

class FragmentNode : public Node
{
  Q_OBJECT
public:
  explicit FragmentNode(Scene* scene);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "FragmentNode");
  QString type() const override { return TYPE; }

  static constexpr auto COLOR_PROPERTY_KEY = "color";

  QString definition() const override;
  static const Detail detail;
};

}  // namespace omm
