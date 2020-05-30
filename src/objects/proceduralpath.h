#pragma once

#include "objects/abstractpath.h"

namespace omm
{

class Scene;

class ProceduralPath : public AbstractPath
{
public:
  explicit ProceduralPath(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "ProceduralPath");
  Flag flags() const override;

  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto COUNT_PROPERTY_KEY = "count";

  void update() override;
  Geom::PathVector paths() const override;

protected:
  void on_property_value_changed(Property* property);

private:
  std::vector<Point> m_points;

};

}  // namespace omm
