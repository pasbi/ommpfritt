#pragma once

#include "objects/object.h"
#include "objects/instance.h"
#include <Qt>
#include <random>

namespace omm
{

class Property;
class Tag;
class Scene;

class Cloner : public Object
{
public:
  explicit Cloner(Scene* scene);
  explicit Cloner(const Cloner& other);
  void draw_object(Painter& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
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
  static constexpr auto ANCHOR_PROPERTY_KEY = "anchor";

  enum class Mode { Linear, Grid, Radial, Path, Script, FillRandom  };
  std::unique_ptr<Object> clone() const override;
  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() const override;
  Mode mode() const;
  bool contains(const Vec2f &pos) const override;
  void update() override;

protected:
  void on_change(AbstractPropertyOwner* subject, int code, Property* property,
                 std::set<const void*> trace) override;
  void on_property_value_changed(Property& property, std::set<const void*> trace) override;

private:
  std::vector<std::unique_ptr<Object>> make_clones();
  std::vector<std::unique_ptr<Object>> copy_children(const std::size_t n);

  double get_t(std::size_t i, const bool inclusive) const;
  void set_linear(Object& object, std::size_t i);
  void set_grid(Object& object, std::size_t i);
  void set_radial(Object& object, std::size_t i);
  void set_path(Object& object, std::size_t i);
  void set_by_script(Object& object, std::size_t i);
  void set_fillrandom(Object& object, std::mt19937 &rng);
  std::vector<std::unique_ptr<Object>> m_clones;
  std::set<Property*> m_clone_dependencies;

};

}  // namespace omm
