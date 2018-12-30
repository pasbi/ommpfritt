#pragma once

#include "tools/tool.h"
#include "objects/object.h"

namespace omm
{

class ObjectTransformationTool : public Tool
{
public:
  using Tool::Tool;
  static constexpr auto TYPE = "ObjectTransformationTool";
  void transform_objects(const ObjectTransformation& transformation);

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos) override;
  bool mouse_press(const arma::vec2& pos) override;
  void mouse_release() override;
};

}  // namespace omm
