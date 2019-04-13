#pragma once

#include "objects/object.h"
#include "objects/instance.h"
#include <Qt>

namespace omm
{

class Property;
class Tag;
class Scene;

class Cloner : public Object
{
public:
  explicit Cloner(Scene* scene);
  void draw_object(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Cloner");
  static constexpr auto MODE_PROPERTY_KEY = "mode";
  static constexpr auto CODE_PROPERTY_KEY = "code";
  static constexpr auto COUNT_PROPERTY_KEY = "count";
  static constexpr auto COUNT_2D_PROPERTY_KEY = "count2d";
  static constexpr auto DISTANCE_2D_PROPERTY_KEY = "distance2d";
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  static constexpr auto PATH_REFERENCE_PROPERTY_KEY = "path";
  static constexpr auto START_PROPERTY_KEY = "start";
  static constexpr auto END_PROPERTY_KEY = "end";
  static constexpr auto BORDER_PROPERTY_KEY = "border";
  static constexpr auto ALIGN_PROPERTY_KEY = "align";
  static constexpr auto SEED_PROPERTY_KEY = "seed";

  enum class Mode { Linear, Grid, Radial, Path, Script, FillRandom  };
  std::unique_ptr<Object> clone() const override;
  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() override;
  Mode mode() const;
  bool contains(const Vec2f &pos) override;

private:
  std::vector<std::unique_ptr<Object>> make_clones();
  std::vector<std::unique_ptr<Object>> copy_children(const std::size_t n);

  double get_t(std::size_t i, const bool inclusive) const;
  void set_linear(Object& object, std::size_t i);
  void set_grid(Object& object, std::size_t i);
  void set_radial(Object& object, std::size_t i);
  void set_path(Object& object, std::size_t i);
  void set_by_script(Object& object, std::size_t i);
  void set_fillrandom(Object& object, std::size_t i);


};

}  // namespace omm
