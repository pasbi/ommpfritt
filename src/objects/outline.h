#pragma once

#include "objects/object.h"
#include "objects/path.h"
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
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Outline");
  static constexpr auto OFFSET_PROPERTY_KEY = "offset";
  static constexpr auto REFERENCE_PROPERTY_KEY = "ref";

  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() const override;
  void update() override;

  Point evaluate(const double t) const override;
  double path_length() const override;
  bool contains(const Vec2f &pos) const override;

protected:
  void on_property_value_changed(Property *property) override;

private:
  std::unique_ptr<Path> m_outline;
  void polish();
};

}  // namespace omm
