#pragma once

#include <vector>
#include <memory>
#include "external/json_fwd.hpp"
#include "geometry/objecttransformation.h"
#include "aspects/propertyowner.h"
#include "abstractfactory.h"
#include "aspects/treeelement.h"
#include "common.h"
#include "renderers/abstractrenderer.h"
#include "scene/list.h"
#include "geometry/point.h"

namespace omm
{

class Scene;
class Property;

class Object
  : public PropertyOwner<AbstractPropertyOwner::Kind::Object>
  , public virtual Serializable
  , public TreeElement<Object>
  , public AbstractFactory<std::string, Object, Scene*>
{
public:
  explicit Object(Scene* scene);
  explicit Object(const Object& other);
  virtual ~Object();

  enum class Visibility { Visible, Hide, HideTree };

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation(const bool skip_root = false) const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation( const ObjectTransformation& globalTransformation,
                                  const bool skip_root = false );

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual void render(AbstractRenderer& renderer, const Style& style) = 0;

  struct RenderOptions
  {
    std::vector<const Style*> styles;
    bool always_visible = true;
    const Style* default_style = nullptr;
  };

  void render_recursive(AbstractRenderer& renderer, const Style& default_style);
  void render_recursive(AbstractRenderer& renderer, const RenderOptions& options);
  virtual BoundingBox bounding_box() = 0;
  BoundingBox recursive_bounding_box();
  std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() override;
  virtual std::unique_ptr<Object> clone() const = 0;
  std::unique_ptr<Object> clone(Scene* scene) const;
  virtual std::unique_ptr<Object> convert();
  Flag flags() const override;
  Scene* scene() const;
  bool is_active() const;
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

  enum class Border { Clamp, Wrap, Hide, Reflect };
  static double apply_border(double t, Border border);
  virtual OrientedPoint evaluate(const double t);
  virtual double path_length();

  void set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t);
  void set_oriented_position(const OrientedPoint& op, const bool align);

protected:
  bool m_draw_children = true;
  void copy_tags(Object& other) const;

private:
  friend class ObjectView;
  Scene* m_scene;
  void set_scene(Scene* scene);
};

void register_objects();
std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
