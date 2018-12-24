#pragma once

#include <vector>
#include <memory>
#include "external/json_fwd.hpp"
#include "geometry/objecttransformation.h"
#include "aspects/propertyowner.h"
#include "abstractfactory.h"
#include "aspects/treeelement.h"
#include "common.h"
#include "aspects/copycreatable.h"
#include "renderers/abstractrenderer.h"
#include "scene/list.h"

namespace omm
{

class Scene;
class Property;

class Object
  : public PropertyOwner<AbstractPropertyOwner::Kind::Object>
  , public virtual Serializable
  , public TreeElement<Object>
  , public CopyCreatable<Object, Scene&>
{
public:
  explicit Object(Scene& scene);
  virtual ~Object();

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation() const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation(const ObjectTransformation& globalTransformation);

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  virtual void render(AbstractRenderer& renderer, const Style& style) const = 0;
  void render_recursive(AbstractRenderer& renderer, const Style& default_style) const;
  virtual BoundingBox bounding_box() const = 0;
  BoundingBox recursive_bounding_box() const;
  std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() override;
  std::unique_ptr<Object> copy() const override;

  Scene& scene;
  List<Tag> tags;

  static constexpr auto TYPE = "Object";
  static constexpr auto TRANSFORMATION_PROPERTY_KEY = "transformation";


protected:
  bool m_draw_children = true;

private:
  friend class ObjectView;
};

void register_objects();
std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
