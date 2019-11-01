#pragma once

#include <vector>
#include <memory>
#include "external/json_fwd.hpp"
#include "geometry/objecttransformation.h"
#include "aspects/propertyowner.h"
#include "abstractfactory.h"
#include "aspects/treeelement.h"
#include "common.h"
#include "renderers/painter.h"
#include "scene/taglist.h"
#include "geometry/point.h"

namespace omm
{

class ObjectTree;
class Scene;
class Property;
class Path;

class PathDeleter
{
public:
  void operator()(Path* path);
};

class Object
  : public PropertyOwner<AbstractPropertyOwner::Kind::Object>
  , public virtual Serializable
  , public TreeElement<Object>
  , public AbstractFactory<QString, Object, Scene*>
{
  Q_OBJECT

  Scene* m_scene;

public:
  using PathUniquePtr = std::unique_ptr<Path, PathDeleter>;

  explicit Object(Scene* scene);
  explicit Object(const Object& other);
  ~Object();

  enum class Visibility { Default, Hidden, Visible };

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation(Space space) const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation( const ObjectTransformation& global_transformation,
                                  Space space );
  virtual void set_global_axis_transformation( const ObjectTransformation& global_transformation,
                                               Space space);

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual void draw_handles(Painter& renderer) const;

  virtual void draw_object(Painter& renderer, const Style& style, Painter::Options options) const;
  void draw_recursive(Painter& renderer, Painter::Options options) const;

  /**
   * @brief bounding_box returns the bounding box in world coordinates
   */
  virtual BoundingBox bounding_box(const ObjectTransformation& transformation) const = 0;
  virtual BoundingBox recursive_bounding_box(const ObjectTransformation &transformation) const;
  std::unique_ptr<Object> repudiate(Object &repudiatee) override;
  Object & adopt(std::unique_ptr<Object> adoptee, const size_t pos) override;
  using TreeElement::adopt;
  virtual std::unique_ptr<Object> clone() const = 0;
  virtual std::unique_ptr<Object> convert() const;
  Flag flags() const override;
  bool is_active() const;
  bool is_visible(bool viewport) const;
  virtual std::vector<const omm::Style*> find_styles() const;

  TagList tags;
  template<typename T, template<typename...> class ContainerT>
  static ContainerT<T*> cast(const ContainerT<Object*> object)
  {
    const auto type_matches = [](const Object* o) { return o->type() == T::TYPE; };
    const auto to_type = [](Object* o) { return static_cast<T*>(o); };
    return ::transform<T*>(::filter_if(object, type_matches), to_type);
  }

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
  virtual Point evaluate(const double t) const;
  virtual double path_length() const;
  virtual bool is_closed() const;
  void set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t,
                            Space space);
  void set_oriented_position(const Point &op, const bool align);
  virtual PathUniquePtr outline(const double offset) const;
  virtual std::vector<Point> points() const;

  virtual bool contains(const Vec2f& pos) const;
  void update_recursive();

  virtual void post_create_hook();

public Q_SLOTS:
  virtual void update();

protected:
  bool m_draw_children = true;
  void copy_tags(Object& other) const;
  void on_property_value_changed(Property* property) override;
  void on_child_added(Object &child) override;
  void on_child_removed(Object &child) override;
  void listen_to_changes(const std::function<Object*()>& get_watched);
  void listen_to_children_changes();

private:
  friend class ObjectView;

public:
  void set_object_tree(ObjectTree& object_tree);
private:
  ObjectTree* m_object_tree = nullptr;

private:
  mutable bool m_visibility_cache_is_dirty = true;
  mutable bool m_visibility_cache_value;
  static std::unique_ptr<Style> m_bounding_box_style;
};

void register_objects();
std::ostream& operator<<(std::ostream& ostream, const Object& object);
std::ostream& operator<<(std::ostream& ostream, const Object* object);

}  // namespace omm
