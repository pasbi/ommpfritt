#pragma once

#include <memory>
#include <vector>
#include "tools/tool.h"
#include "tools/handles/handle.h"

namespace omm {

template<typename PositionVariant>
class TransformationTool : public Tool
{
public:
  TransformationTool(Scene& scene, std::vector<std::unique_ptr<Handle>> handles);
  void transform_objects(const ObjectTransformation& transformation);
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos) override;
  bool mouse_press( const arma::vec2& pos,
                    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) override;
  void mouse_release(const arma::vec2& pos) override;
  virtual ObjectTransformation transformation() const;
  void draw(AbstractRenderer& renderer) const override;

  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";

private:
  PositionVariant m_position_variant;
};

}  // namespace omm
