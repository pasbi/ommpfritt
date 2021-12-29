#pragma once

#include "objects/object.h"
#include "properties/propertygroups/markerproperties.h"

namespace omm
{
class Tip : public Object
{
public:
  explicit Tip(Scene* scene);
  Tip(const Tip& other);
  Tip(Tip&&) = delete;
  Tip& operator=(Tip&&) = delete;
  Tip& operator=(const Tip&) = delete;
  ~Tip() override = default;

  QString type() const override
  {
    return TYPE;
  }
  void on_property_value_changed(Property* property) override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Tip");

protected:
  PathVector compute_path_vector() const override;

private:
  MarkerProperties m_marker_properties;
};

}  // namespace omm
