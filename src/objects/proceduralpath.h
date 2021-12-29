#pragma once

#include "objects/object.h"
#include "geometry/point.h"

namespace omm
{
class Scene;

class ProceduralPath : public Object
{
public:
  explicit ProceduralPath(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ProceduralPath");
  Flag flags() const override;

  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto COUNT_PROPERTY_KEY = "count";

  void update() override;
  PathVector compute_path_vector() const override;

protected:
  void on_property_value_changed(Property* property) override;

private:
  std::deque<std::deque<Point>> m_points;
  std::vector<std::set<int>> m_joined_points;
};

}  // namespace omm
