#pragma once

#include <memory>
#include <vector>
#include "tools/tool.h"
#include "tools/handles/handle.h"

namespace omm {

class ObjectsTool : public Tool
{
public:
  ObjectsTool(Scene& scene, std::vector<std::unique_ptr<Handle>> handles);
  ObjectTransformation transformation() const;
  arma::vec2 map_to_tool_local(const arma::vec2& pos) const;
  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";

  void transform_objects(const ObjectTransformation& transformation);
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos) override;
  bool mouse_press(const arma::vec2& pos) override;
  void mouse_release() override;
  void draw(AbstractRenderer& renderer) const override;
};

}  // namespace omm
