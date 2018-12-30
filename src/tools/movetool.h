#pragma once

#include "tools/tool.h"
#include "objects/object.h"

namespace omm
{

class MoveTool : public Tool
{
public:
  explicit MoveTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "MoveTool";
  void transform_objects(const ObjectTransformation& transformation);

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos) override;
  bool mouse_press(const arma::vec2& pos) override;
  void mouse_release() override;
};

}  // namespace omm
