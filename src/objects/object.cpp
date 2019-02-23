#include "objects/object.h"

#include <cassert>
#include <algorithm>
#include <map>
#include <functional>
#include <QObject>

#include "tags/tag.h"
#include "properties/stringproperty.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"
#include "properties/referenceproperty.h"
#include "common.h"
#include "serializers/jsonserializer.h"
#include "renderers/style.h"
#include "tags/styletag.h"
#include "scene/contextes.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "properties/vectorproperty.h"

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

std::vector<const omm::Style*> find_styles(const omm::Object& object)
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      const auto* property_owner = tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)
                                       .value<omm::ReferenceProperty::value_type>();
      assert(  property_owner == nullptr
            || property_owner->kind() == omm::AbstractPropertyOwner::Kind::Style );
      return static_cast<const omm::Style*>(property_owner);
    } else {
      return nullptr;
    }
  };

  const auto tags = object.tags.ordered_items();
  return ::filter_if(::transform<const omm::Style*>(tags, get_style), ::is_not_null);
}

}  // namespace

namespace omm
{

Object::Object(Scene* scene)
  : TreeElement(nullptr)
  , tags(scene)
{
  set_scene(scene);

  add_property<OptionsProperty>(IS_VISIBLE_PROPERTY_KEY, true)
    .set_options({ QObject::tr("visible").toStdString(), QObject::tr("hidden").toStdString(),
      QObject::tr("hide tree").toStdString() })
    .set_label(QObject::tr("").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<BoolProperty>(IS_ACTIVE_PROPERTY_KEY, true)
    .set_label(QObject::tr("active").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>").toStdString())
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<FloatVectorProperty>(POSITION_PROPERTY_KEY, arma::vec2{ 0.0, 0.0 })
    .set_label(QObject::tr("pos").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<FloatVectorProperty>(SCALE_PROPERTY_KEY, arma::vec2{ 1.0, 1.0 })
    .set_step(arma::vec2{ 0.1, 0.1 })
    .set_label(QObject::tr("scale").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<FloatProperty>(ROTATION_PROPERTY_KEY, 0.0)
    .set_multiplier(180.0 / M_PI)
    .set_label(QObject::tr("rotation").toStdString())
    .set_category(QObject::tr("basic").toStdString());

  add_property<FloatProperty>(SHEAR_PROPERTY_KEY, 0.0)
    .set_step(0.01)
    .set_label(QObject::tr("shear").toStdString())
    .set_category(QObject::tr("basic").toStdString());
}

Object::Object(const Object& other)
  : PropertyOwner(other)
  , TreeElement(other)
  , tags(other.tags)
  , m_draw_children(other.m_draw_children)
{
  set_scene(other.m_scene);
  for (Tag* tag : tags.items()) {
    tag->owner = this;
  }
}

Object::~Object() { }

void Object::set_scene(Scene* scene)
{
  tags.set_scene(scene);
  m_scene = scene;
}

ObjectTransformation Object::transformation() const
{
  return ObjectTransformation(
    property(POSITION_PROPERTY_KEY).value<VectorPropertyValueType<arma::vec2>>(),
    property(SCALE_PROPERTY_KEY).value<VectorPropertyValueType<arma::vec2>>(),
    property(ROTATION_PROPERTY_KEY).value<double>(),
    property(SHEAR_PROPERTY_KEY).value<double>()
  );
}

ObjectTransformation Object::global_transformation() const
{
  if (is_root()) {
    return transformation();
  } else {
    // TODO caching could gain some speed
    //  invalidate cache if local transformation is set or parent changes
    return parent().global_transformation().apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property(POSITION_PROPERTY_KEY).set(transformation.translation());
  property(SCALE_PROPERTY_KEY).set(transformation.scaling());
  property(ROTATION_PROPERTY_KEY).set(transformation.rotation());
  property(SHEAR_PROPERTY_KEY).set(transformation.shearing());
}

void Object::set_global_transformation(const ObjectTransformation& global_transformation)
{
  ObjectTransformation local_transformation;
  if (is_root()) {
    local_transformation = global_transformation;
  } else {
    try {
      local_transformation =
        parent().global_transformation().inverted().apply(global_transformation);
    } catch (const std::runtime_error& e) {
      assert(false);
    }
  }
  set_transformation(local_transformation);
}

void Object::transform(const ObjectTransformation& transformation)
{
  set_transformation(transformation.apply(this->transformation()));
}

std::ostream& operator<<(std::ostream& ostream, const Object& object)
{
  ostream << object.type() << "[" << object.name() << "]";
  return ostream;
}

void Object::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  serializer.start_array(n_children(), children_pointer);
  for (size_t i = 0; i < n_children(); ++i) {
    const auto& child = this->child(i);
    const auto child_pointer = make_pointer(children_pointer, i);
    serializer.set_value(child.type(), make_pointer(child_pointer, TYPE_POINTER));
    child.serialize(serializer, child_pointer);
  }
  serializer.end_array();

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  serializer.start_array(tags.size(), tags_pointer);
  for (size_t i = 0; i < tags.size(); ++i) {
    const auto& tag = tags.item(i);
    const auto tag_pointer = make_pointer(tags_pointer, i);
    serializer.set_value(tag.type(), make_pointer(tag_pointer, TYPE_POINTER));
    tag.serialize(serializer, tag_pointer);
  }
  serializer.end_array();
}

void Object::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  size_t n_children = deserializer.array_size(children_pointer);
  for (size_t i = 0; i < n_children; ++i) {
    const auto child_pointer = make_pointer(children_pointer, i);
    const auto child_type = deserializer.get_string(make_pointer(child_pointer, TYPE_POINTER));
    try {
      auto child = Object::make(child_type, static_cast<Scene*>(m_scene));
      child->deserialize(deserializer, child_pointer);

      // TODO adopt sets the global transformation which is reverted by setting the local
      //  transformation immediately afterwards. That can be optimized.
      const auto t = child->transformation();
      adopt(std::move(child)).set_transformation(t);
    } catch (std::out_of_range& e) {
      const auto message = QObject::tr("Failed to retrieve object type '%1'.")
                            .arg(QString::fromStdString(child_type)).toStdString();
      LOG(ERROR) << message;
      throw AbstractDeserializer::DeserializeError(message);
    }
  }

  const auto tags_pointer = make_pointer(root, TAGS_POINTER);
  size_t n_tags = deserializer.array_size(tags_pointer);
  std::vector<std::unique_ptr<Tag>> tags;
  tags.reserve(n_tags);
  for (size_t i = 0; i < n_tags; ++i) {
    const auto tag_pointer = make_pointer(tags_pointer, i);
    const auto tag_type = deserializer.get_string(make_pointer(tag_pointer, TYPE_POINTER));
    auto tag = Tag::make(tag_type, *this);
    tag->deserialize(deserializer, tag_pointer);
    tags.push_back(std::move(tag));
  }
  this->tags.set(std::move(tags));
}

void Object::render_recursive(AbstractRenderer& renderer, const Style& default_style)
{
  const auto visibility = property(IS_VISIBLE_PROPERTY_KEY).value<Visibility>();
  if (visibility == Visibility::Visible) {
    const auto styles = find_styles(*this);
    for (const auto* style : styles) {
      render(renderer, *style);
    }
    if (styles.size() == 0) {
      render(renderer, default_style);
    }
  }

  if (visibility != Visibility::HideTree && m_draw_children) {
    for (const auto& child : children()) {
      renderer.push_transformation(child->transformation());
      child->render_recursive(renderer, default_style);
      renderer.pop_transformation();
    }
  }
}

BoundingBox Object::recursive_bounding_box()
{
  auto bounding_box = this->bounding_box();

  for (const auto& child : children()) {
    bounding_box = bounding_box.merge(child->recursive_bounding_box());
  }
  return transformation().apply(bounding_box);
}

std::unique_ptr<AbstractRAIIGuard> Object::acquire_set_parent_guard()
{
  class SetParentGuard : public AbstractRAIIGuard
  {
  public:
    explicit SetParentGuard(Object& self)
      : m_self(self), m_global_transformation(m_self.global_transformation()) { }

    ~SetParentGuard() { m_self.set_global_transformation(m_global_transformation); }

  private:
    Object& m_self;
    const ObjectTransformation m_global_transformation;
  };
  return std::make_unique<SetParentGuard>(*this);
}

std::unique_ptr<Object> Object::clone(Scene* scene) const
{
  auto clone = this->clone();
  clone->m_scene = scene;
  return clone;
}

std::unique_ptr<Object> Object::convert() { return clone(); }
AbstractPropertyOwner::Flag Object::flags() const { return Flag::None; }
Scene* Object::scene() const { return m_scene; }

void Object::copy_tags(Object& other) const
{
  for (const Tag* tag : tags.items()) {
    ListOwningContext<Tag> context(tag->clone(), other.tags);
    other.tags.insert(context);
  }
}

double Object::apply_border(double t, Border border)
{
  switch (border) {
  case Border::Clamp: return std::min(1.0, std::max(0.0, t));
  case Border::Wrap: return fmod(fmod(t, 1.0) + 1.0, 1.0);
  case Border::Hide: return apply_border(t, Border::Clamp) == t ? t : -1.0;
  case Border::Reflect: {
    const bool flip = int(t / 1.0) % 2 == 1;
    t = apply_border(t, Border::Wrap);
    return flip ? (1.0-t) : t;
  }
  default:
    assert(false); return 0.0;
  }
}
OrientedPoint Object::evaluate(const double t) { return OrientedPoint(); }
double Object::path_length() { return -1.0; }

void Object::set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t)
{
  if (path != nullptr && path->kind() == AbstractPropertyOwner::Kind::Object) {
    auto* path_object = static_cast<Object*>(path);
    if (!path_object->is_descendant_of(*this)) {
      const auto location = path_object->evaluate(std::clamp(t, 0.0, 1.0));
      const auto global_location = path_object->global_transformation().apply(location);
      set_oriented_position(global_location, align);
    } else {
      LOG(WARNING) << "cycle.";
    }
  }
}

void Object::set_oriented_position(const OrientedPoint& op, const bool align)
{
  auto transformation = global_transformation();
  if (align) { transformation.set_rotation(op.rotation); }
  transformation.set_translation(op.position);
  set_global_transformation(transformation);
}

bool Object::is_active() const { return property(IS_ACTIVE_PROPERTY_KEY).value<bool>(); }

}  // namespace omm
