#pragma once

#include "objects/abstractproceduralpath.h"
#include <Qt>

namespace omm
{

class Scene;

class Ellipse : public AbstractProceduralPath
{
public:
  explicit Ellipse(Scene* scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Ellipse");
  std::unique_ptr<Object> clone() const override;
  Flag flags() const override;

  static constexpr auto RADIUS_PROPERTY_KEY = "r";
  static constexpr auto CORNER_COUNT_PROPERTY_KEY = "n";
  static constexpr auto SMOOTH_PROPERTY_KEY = "smooth";

private:
  std::vector<Point> points() const override;
  bool is_closed() const override;
};

}  // namespace omm
