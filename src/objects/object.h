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

  enum class Flag { None = 0x0, Convertable = 0x1 };

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation() const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation(const ObjectTransformation& globalTransformation);

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual void render(AbstractRenderer& renderer, const Style& style) = 0;
  void render_recursive(AbstractRenderer& renderer, const Style& default_style);
  virtual BoundingBox bounding_box() = 0;
  BoundingBox recursive_bounding_box();
  std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() override;
  virtual std::unique_ptr<Object> clone() const = 0;
  std::unique_ptr<Object> clone(Scene* scene) const;
  virtual std::unique_ptr<Object> convert();
  virtual Flag flags() const;
  Scene* scene() const;
  bool is_active() const;

  List<Tag> tags;
  template<typename T, template<typename...> class ContainerT>
  static ContainerT<T*> cast(const ContainerT<Object*> object)
  {
    const auto type_matches = [](const Object* o) { return o->type() == T::TYPE; };
    const auto to_type = [](Object* o) { return static_cast<T*>(o); };
    return ::transform<T*>(::filter_if(object, type_matches), to_type);
  }

  static constexpr auto TYPE = "Object";
  static constexpr auto TRANSFORMATION_PROPERTY_KEY = "transformation";
  static constexpr auto IS_ACTIVE_PROPERTY_KEY = "is_active";

  enum class Border { Clamp, Wrap, Hide, Reflect };
  static double apply_border(double t, Border border);
  virtual OrientedPoint evaluate(const double t);
  virtual double path_length();

  void set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t);
  void set_oriented_position(const OrientedPoint& op, const bool align);

protected:
  bool m_draw_children = true;
  void copy_properties(Object& other) const;
  void copy_tags(Object& other) const;

private:
  friend class ObjectView;
  Scene* m_scene;
  void set_scene(Scene* scene);
};

void register_objects();
std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm

template<> struct EnableBitMaskOperators<omm::Object::Flag> : std::true_type {};
