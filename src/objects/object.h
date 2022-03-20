#pragma once

#include "abstractfactory.h"
#include "aspects/propertyowner.h"
#include "aspects/treeelement.h"
#include "cachedgetter.h"
#include "common.h"
#include "external/json_fwd.hpp"
#include "geometry/objecttransformation.h"
#include "scene/taglist.h"
#include <memory>
#include <vector>
#include <QPainterPath>
#include <deque>

namespace omm
{

class ObjectTree;
class Painter;
class Point;
class Property;
class Scene;
struct PainterOptions;
class PathVector;

class Object
    : public PropertyOwner<Kind::Object>
    , public TreeElement<Object>
    , public AbstractFactory<QString, true, Object, Scene*>
{
  Q_OBJECT

public:
  explicit Object(Scene* scene);
  explicit Object(const Object& other);
  ~Object() override;
  Object(Object&&) = delete;
  Object& operator=(Object&&) = delete;
  Object& operator=(const Object&) = delete;

  enum class Visibility { Default, Hidden, Visible };

  void transform(const ObjectTransformation& transformation);
  void set_virtual_parent(const Object* parent);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation(Space space) const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation(const ObjectTransformation& global_transformation, Space space);
  virtual void set_global_axis_transformation(const ObjectTransformation& global_transformation,
                                              Space space);
  bool is_transformation_property(const Property& property) const;

  void serialize(serialization::SerializerWorker& worker) const override;
  void deserialize(serialization::DeserializerWorker& worker) override;
  virtual void draw_handles(Painter& renderer) const;

  virtual void
  draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const;
  void draw_recursive(Painter& renderer, PainterOptions options) const;

  /**
   * @brief bounding_box returns the bounding box in world coordinates
   */
  virtual BoundingBox bounding_box(const ObjectTransformation& transformation) const;
  virtual BoundingBox recursive_bounding_box(const ObjectTransformation& transformation) const;
  std::unique_ptr<Object> repudiate(Object& repudiatee) override;
  Object& adopt(std::unique_ptr<Object> adoptee, std::size_t pos) override;
  using TreeElement::adopt;

  virtual std::unique_ptr<Object> convert(bool& keep_children) const;

  Flag flags() const override;
  bool is_active() const;
  bool is_visible(bool viewport) const;
  virtual std::deque<const Style*> find_styles() const;

  virtual Point pos(const Geom::PathVectorTime& t) const;
  virtual bool contains(const Vec2f& point) const;

private:
  /**
   * @brief paths re-implement this if you're class has geometry that can be expressed as paths.
   *  The drawing, converting and other nice features will be handled automatically.
   * @note use `Object::geom_paths` or `Object::painter_path` to access the paths.
   * @return the paths.
   */
  virtual PathVector compute_path_vector() const;

public:
  enum class Interpolation { Natural, Distance };
  Geom::PathVectorTime compute_path_vector_time(double t,
                                                Interpolation = Interpolation::Natural) const;
  Geom::PathVectorTime
  compute_path_vector_time(int path_index, double t, Interpolation = Interpolation::Natural) const;

private:
  class CachedGeomPathVectorGetter;
  std::unique_ptr<CachedGeomPathVectorGetter> m_cached_geom_path_vector_getter;
public:
  const PathVector& path_vector() const;

  TagList tags;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP(ANY_TR_CONTEXT, "Object");
  static constexpr auto VISIBILITY_PROPERTY_KEY = "visibility";
  static constexpr auto VIEWPORT_VISIBILITY_PROPERTY_KEY = "viewport-visibility";
  static constexpr auto IS_ACTIVE_PROPERTY_KEY = "is_active";
  static constexpr auto POSITION_PROPERTY_KEY = "position";
  static constexpr auto SCALE_PROPERTY_KEY = "scale";
  static constexpr auto ROTATION_PROPERTY_KEY = "rotation";
  static constexpr auto SHEAR_PROPERTY_KEY = "shear";
  static constexpr auto HIERARCHY_CHANGED = 1;
  static constexpr auto TAG_CHANGED = 2;

  enum class Border { Clamp, Wrap, Hide, Reflect };
  static double apply_border(double t, Border border);

  void update_recursive();
  QString tree_path() const;

  virtual void post_create_hook();

public:
  virtual void update();

public:
  void copy_tags(Object& other) const;

protected:
  bool m_draw_children = true;
  void on_property_value_changed(Property* property) override;
  void on_child_added(Object& child) override;
  void on_child_removed(Object& child) override;
  void listen_to_changes(const std::function<Object*()>& get_watched);
  void listen_to_children_changes();

private:
  friend class ObjectView;

public:
  void set_object_tree(ObjectTree& object_tree);
  void set_position_on_path(const Object& path, bool align, const Geom::PathVectorTime& t);
  void set_oriented_position(const Point& op, bool align);

  QString to_string() const override;

private:
  ObjectTree* m_object_tree = nullptr;
  const Object* m_virtual_parent = nullptr;

private:
  mutable bool m_visibility_cache_is_dirty = true;
  mutable bool m_visibility_cache_value = false;
  static const QPen m_bounding_box_pen;
  static const QBrush m_bounding_box_brush;

protected:
  static PathVector join(const std::vector<Object*>& objects);
};

}  // namespace omm
