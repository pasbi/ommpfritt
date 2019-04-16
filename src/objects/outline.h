#pragma once

#include "objects/object.h"
#include <Qt>

namespace omm
{

class Property;
class Tag;
class Scene;

class Outline : public Object
{
public:
  explicit Outline(Scene* scene);
  Outline(const Outline& other);
  void draw_object(AbstractRenderer& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Outline");
  static constexpr auto OFFSET_PROPERTY_KEY = "offset";
  static constexpr auto REFERENCE_PROPERTY_KEY = "ref";

  std::unique_ptr<Object> clone() const override;
  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() const override;
  void update() override;

private:
  std::unique_ptr<Object> m_outline;
};

}  // namespace omm
