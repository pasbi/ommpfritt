#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{
class Scene;

class Boolean : public Object
{
public:
  explicit Boolean(Scene* scene);
  Boolean(const Boolean& other);
  Boolean(Boolean&&) = delete;
  Boolean& operator=(Boolean&&) = delete;
  Boolean& operator=(const Boolean&) = delete;

  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Boolean");
  void update() override;

private:
  static constexpr auto MODE_PROPERTY_KEY = "mode";
  void on_property_value_changed(Property* property) override;
  void polish();
  bool is_closed() const override;
  Geom::PathVector paths() const override;
};

}  // namespace omm
