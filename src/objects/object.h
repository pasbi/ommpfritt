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
#include "scene/list.h"
#include "geometry/point.h"

namespace omm
{

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
  , public AbstractFactory<std::string, Object, Scene*>
{
  Q_OBJECT

public:
  using PathUniquePtr = std::unique_ptr<Path, PathDeleter>;

  explicit Object(Scene* scene);
  explicit Object(const Object& other);

  enum class Visibility { Visible, Hide, HideTree };

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation(const bool skip_root = false) const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation( const ObjectTransformation& global_transformation,
                                  const bool skip_root = false );
  virtual void set_global_axis_transformation( const ObjectTransformation& global_transformation,
                                               const bool skip_root = false );

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual void draw_object(Painter& renderer, const Style& style) const;
  virtual void draw_handles(Painter& renderer) const;

  static Style m_bounding_box_style;

  struct RenderOptions
  {
    std::vector<const Style*> styles;
    bool always_visible = true;
    const Style* default_style = nullptr;
  };

  void draw_recursive(Painter& renderer, const Style& default_style) const;
  void draw_recursive(Painter& renderer, const RenderOptions& options) const;

  /**
   * @brief bounding_box returns the bounding box in world coordinates
   */
  virtual BoundingBox bounding_box(const ObjectTransformation& transformation) const = 0;
  BoundingBox recursive_bounding_box(const ObjectTransformation &transformation) const;
  std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() override;
  virtual std::unique_ptr<Object> clone() const = 0;
  std::unique_ptr<Object> clone(Scene* scene) const;
  virtual std::unique_ptr<Object> convert() const;
  Flag flags() const override;
  Scene* scene() const;
  bool is_active() const;
  bool is_visible() const;
  Visibility visibility() const;
  virtual std::vector<const omm::Style*> find_styles() const;

  List<Tag> tags;
  template<typename T, template<typename...> class ContainerT>
  static ContainerT<T*> cast(const ContainerT<Object*> object)
  {
    const auto type_matches = [](const Object* o) { return o->type() == T::TYPE; };
    const auto to_type = [](Object* o) { return static_cast<T*>(o); };
    return ::transform<T*>(::filter_if(object, type_matches), to_type);
  }

  static constexpr auto TYPE = QT_TRANSLATE_NOOP(ANY_TR_CONTEXT, "Object");
  static constexpr auto IS_VISIBLE_PROPERTY_KEY = "is_visible";
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
                            const bool skip_root);
  void set_oriented_position(const Point &op, const bool align);
  virtual PathUniquePtr outline(const double offset) const;
  virtual std::vector<Point> points() const;

  virtual bool contains(const Vec2f& pos) const;
  void update_recursive();

  virtual void post_create_hook();

public Q_SLOTS:
  virtual void update();

Q_SIGNALS:
  void appearance_changed(Object*);
  void transformation_changed(Object*);
  void child_appearance_changed(Object*);
  void child_transformation_changed(Object*);

protected:
  bool m_draw_children = true;
  void copy_tags(Object& other) const;
  void on_property_value_changed(Property* property) override;

  void on_child_added(Object &child) override;
  void on_child_removed(Object &child) override;
private:
  friend class ObjectView;
  Scene* m_scene;
  void set_scene(Scene* scene);
};

void register_objects();
std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
