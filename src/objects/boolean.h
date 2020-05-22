#pragma once

#include "objects/abstractproceduralpath.h"
#include <Qt>

namespace omm
{

class Scene;

class Boolean : public AbstractProceduralPath
{
public:
  explicit Boolean(Scene* scene);
  Boolean(const Boolean& other);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Boolean");
  Flag flags() const override;
  void update() override;

private:
  static constexpr auto MODE_PROPERTY_KEY = "mode";
  void on_property_value_changed(Property *property) override;
  void polish();
  std::vector<Point> points() const override;
  bool is_closed() const override;
};

}  // namespace omm
