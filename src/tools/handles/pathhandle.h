#pragma once

#include <armadillo>
#include "tools/handles/handle.h"

namespace omm
{

class Path;

class PathHandle : public Handle
{
public:
  explicit PathHandle(Path& point);
  bool contains(const arma::vec2& candidate) const override;
  void draw(omm::AbstractRenderer& renderer) const override;

private:
  Path& m_path;
};

}  // namespace omm
