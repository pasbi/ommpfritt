#include "tags/tag.h"

namespace omm
{

class ScriptTag : public Tag
{
public:
  explicit ScriptTag();
  ~ScriptTag();
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ScriptTag";
  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto RUN_PROPERTY_KEY = "run";
  bool run() const override;
  void on_property_value_changed(Property& property) override;
};

}  // namespace omm
