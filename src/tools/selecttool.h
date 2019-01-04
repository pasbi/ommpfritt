#pragma once

#include "tools/tool.h"
#include "objects/object.h"

namespace omm
{

class SelectTool : public Tool
{
public:
  explicit SelectTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "SelectTool";

  bool mouse_press(const arma::vec2& pos) override;
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";

  void update_selection(Object* o, std::set<AbstractPropertyOwner*>& old_selection);

};

}  // namespace omm
