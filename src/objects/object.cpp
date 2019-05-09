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
#include "logging.h"
#include "objects/path.h"
#include "scene/scene.h"

namespace
{

static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

}  // namespace

namespace omm
{

Style Object::m_bounding_box_style = ContourStyle(omm::Colors::BLACK, 1.0);

Object::Object(Scene* scene)
{
  set_scene(scene);

  static const auto category = QObject::tr("basic").toStdString();
  add_property<OptionsProperty>(IS_VISIBLE_PROPERTY_KEY, true)
    .set_options({ QObject::tr("visible").toStdString(), QObject::tr("hidden").toStdString(),
      QObject::tr("hide tree").toStdString() })
    .set_label(QObject::tr("").toStdString())
    .set_category(category);

  add_property<BoolProperty>(IS_ACTIVE_PROPERTY_KEY, true)
    .set_label(QObject::tr("active").toStdString())
    .set_category(category);

  add_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>").toStdString())
    .set_label(QObject::tr("Name").toStdString())
    .set_category(category);

  add_property<FloatVectorProperty>(POSITION_PROPERTY_KEY, Vec2f(0.0, 0.0))
    .set_label(QObject::tr("pos").toStdString())
    .set_category(category);

  add_property<FloatVectorProperty>(SCALE_PROPERTY_KEY, Vec2f(1.0, 1.0))
    .set_step(Vec2f(0.1, 0.1))
    .set_label(QObject::tr("scale").toStdString())
    .set_category(category);

  add_property<FloatProperty>(ROTATION_PROPERTY_KEY, 0.0)
    .set_multiplier(180.0 / M_PI)
    .set_label(QObject::tr("rotation").toStdString())
    .set_category(category);

  add_property<FloatProperty>(SHEAR_PROPERTY_KEY, 0.0)
    .set_step(0.01)
    .set_label(QObject::tr("shear").toStdString())
    .set_category(category);

  QObject::connect(&tags, &List<Tag>::item_changed, [this]() {
    on_change(this, TAG_CHANGED, nullptr);
  });

  QObject::connect(&tags, &List<Tag>::structure_changed, [this]() {
    on_change(this, TAG_CHANGED, nullptr);
    m_scene->invalidate();
  });
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

  QObject::connect(&tags, &List<Tag>::item_changed, [this]() {
    on_change(this, TAG_CHANGED, nullptr);
  });

  QObject::connect(&tags, &List<Tag>::structure_changed, [this]() {
    on_change(this, TAG_CHANGED, nullptr);
    m_scene->invalidate();
  });
}

void Object::set_scene(Scene* scene)
{
  m_scene = scene;
}

ObjectTransformation Object::transformation() const
{
  return ObjectTransformation(
    property(POSITION_PROPERTY_KEY)->value<Vec2f>(),
    property(SCALE_PROPERTY_KEY)->value<Vec2f>(),
    property(ROTATION_PROPERTY_KEY)->value<double>(),
    property(SHEAR_PROPERTY_KEY)->value<double>()
  );
}

ObjectTransformation Object::global_transformation(const bool skip_root) const
{
  if (is_root() || (skip_root && parent().is_root())) {
    return transformation();
  } else {
    // TODO caching could gain some speed
    //  invalidate cache if local transformation is set or parent changes
    return parent().global_transformation(skip_root).apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property(POSITION_PROPERTY_KEY)->set(transformation.translation());
  property(SCALE_PROPERTY_KEY)->set(transformation.scaling());
  property(ROTATION_PROPERTY_KEY)->set(transformation.rotation());
  property(SHEAR_PROPERTY_KEY)->set(transformation.shearing());
}

void Object
::set_global_transformation(const ObjectTransformation& global_transformation, bool skip_root)
{
  ObjectTransformation local_transformation;
  if (is_root() || (skip_root && parent().is_root())) {
    local_transformation = global_transformation;
  } else {
    try {
      local_transformation =
        parent().global_transformation(skip_root).inverted().apply(global_transformation);
    } catch (const std::runtime_error&) {
      assert(false);
    }
  }
  set_transformation(local_transformation);
}


void Object::set_global_axis_transformation( const ObjectTransformation& global_transformation,
                                             const bool skip_root )
{
  const auto get_glob_trans = [skip_root](const auto* c) {
    return c->global_transformation(skip_root);
  };
  const auto child_transformations = ::transform<ObjectTransformation>(children(), get_glob_trans);
  set_global_transformation(global_transformation, skip_root);
  for (std::size_t i = 0; i < child_transformations.size(); ++i) {
    children()[i]->set_global_transformation(child_transformations[i], skip_root);
  }
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
    } catch (std::out_of_range&) {
      const auto message = QObject::tr("Failed to retrieve object type '%1'.")
                            .arg(QString::fromStdString(child_type)).toStdString();
      LERROR << message;
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

void Object::draw_recursive(AbstractRenderer& renderer, const Style& default_style) const
{
  RenderOptions options;
  options.styles = find_styles();
  options.default_style = &default_style;
  options.always_visible = false;
  draw_recursive(renderer, options);
}

void Object::draw_recursive(AbstractRenderer& renderer, const RenderOptions& options) const
{
  renderer.push_transformation(transformation());
  const auto visibility = property(IS_VISIBLE_PROPERTY_KEY)->value<Visibility>();
  const bool is_visible = options.always_visible || visibility == Visibility::Visible;
  const bool is_enabled = !!(renderer.category_filter & AbstractRenderer::Category::Objects);
  if (is_enabled && is_visible) {
    for (const auto* style : options.styles) {
      draw_object(renderer, *style);
    }
    if (options.styles.size() == 0) {
      draw_object(renderer, *options.default_style);
    }
  }

  if (!!(renderer.category_filter & AbstractRenderer::Category::BoundingBox)) {
    renderer.draw_rectangle(bounding_box(), m_bounding_box_style);
  }

  if (!!(renderer.category_filter & AbstractRenderer::Category::Handles)) {
    draw_handles(renderer);
  }

  if (visibility != Visibility::HideTree && m_draw_children) {
    for (const auto& child : children()) {
      child->draw_recursive(renderer, *options.default_style);
    }
  }
  renderer.pop_transformation();
}

BoundingBox Object::recursive_bounding_box() const
{
  auto bounding_box = this->bounding_box();

  for (const auto& child : children()) {
    bounding_box |= child->recursive_bounding_box();
  }
  return transformation().apply(bounding_box);
}

std::unique_ptr<AbstractRAIIGuard> Object::acquire_set_parent_guard()
{
  class SetParentGuard : public AbstractRAIIGuard
  {
  public:
    explicit SetParentGuard(Object& self)
      : m_self(self), m_global_transformation(m_self.global_transformation(true)) { }

    ~SetParentGuard() { m_self.set_global_transformation(m_global_transformation, true); }

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

std::unique_ptr<Object> Object::convert() const { return clone(); }
AbstractPropertyOwner::Flag Object::flags() const { return Flag::None; }
Scene* Object::scene() const { return m_scene; }

void Object::copy_tags(Object& other) const
{
  for (const Tag* tag : tags.items()) {
    ListOwningContext<Tag> context(tag->clone(), other.tags);
    other.tags.insert(context);
  }
}

void Object::on_change(AbstractPropertyOwner* subject, int what, Property* property)
{
  if (!is_root()) {
    parent().on_change(subject, what, property);
  }
  AbstractPropertyOwner::on_change(subject, what, property);
}

void Object::on_children_changed()
{
  on_change(this, HIERARCHY_CHANGED, nullptr);
  TreeElement::on_children_changed();
}

void Object::on_property_value_changed(Property &property)
{
  if (property.type() == ReferenceProperty::TYPE) {
    Object* reference = kind_cast<Object*>(property.value<AbstractPropertyOwner*>());
    if (reference != nullptr && reference->is_ancestor_of(*this)) {
      return; // break the cycle!
    }
  }
  PropertyOwner::on_property_value_changed(property);
}

void Object::post_create_hook() { }

double Object::apply_border(double t, Border border)
{
  switch (border) {
  case Border::Clamp: return std::min(1.0, std::max(0.0, t));
  case Border::Wrap: return fmod(fmod(t, 1.0) + 1.0, 1.0);
  case Border::Hide: return (t >= 0.0 && t <= 1.0) ? t : -1.0;
  case Border::Reflect: {
    const bool flip = int(t / 1.0) % 2 == 1;
    t = apply_border(t, Border::Wrap);
    return flip ? (1.0-t) : t;
  }
  }
  Q_UNREACHABLE();
}

Point Object::evaluate(const double t) const
{
  Q_UNUSED(t)
  return Point();
}

double Object::path_length() const { return -1.0; }
bool Object::is_closed() const { return false; }

void Object::set_position_on_path(AbstractPropertyOwner* path, const bool align, const double t)
{
  if (path != nullptr && path->kind() == AbstractPropertyOwner::Kind::Object) {
    auto* path_object = static_cast<Object*>(path);
    if (!path_object->is_ancestor_of(*this)) {
      const auto location = path_object->evaluate(std::clamp(t, 0.0, 1.0));
      const auto global_location = path_object->global_transformation(true).apply(location);
      set_oriented_position(global_location, align);
    } else {
      LWARNING << "cycle.";
    }
  }
}

void Object::set_oriented_position(const Point& op, const bool align)
{
  auto transformation = global_transformation();
  if (align) { transformation.set_rotation(op.rotation()); }
  transformation.set_translation(op.position);
  set_global_transformation(transformation);
}


bool Object::is_active() const { return property(IS_ACTIVE_PROPERTY_KEY)->value<bool>(); }

bool Object::is_visible() const
{
  const Object* o = this;
  if (o->visibility() == Visibility::Visible) {
    while (!o->is_root()) {
      o = &o->parent();
      if (o->visibility() == Visibility::HideTree) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

Object::Visibility Object::visibility() const
{
  return property(IS_VISIBLE_PROPERTY_KEY)->value<Visibility>();
}

std::vector<const omm::Style*> Object::find_styles() const
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      const auto* property_owner = tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)
                                       ->value<omm::ReferenceProperty::value_type>();
      assert(  property_owner == nullptr
            || property_owner->kind() == omm::AbstractPropertyOwner::Kind::Style );
      return static_cast<const omm::Style*>(property_owner);
    } else {
      return nullptr;
    }
  };

  const auto tags = this->tags.ordered_items();
  return ::filter_if(::transform<const omm::Style*>(tags, get_style), ::is_not_null);
}

bool Object::contains(const Vec2f &point) const
{
  Q_UNUSED(point)
  return false;
}

void Object::update_recursive()
{
  // it's important to first update the children because of the way e.g. Cloner does its caching.
  for (auto* child : children()) {
    child->update_recursive();
  }
  update();
}

void Object::update() { }
void Object::draw_object(AbstractRenderer&, const Style&) const {}
void Object::draw_handles(AbstractRenderer&) const {}
Object::PathUniquePtr Object::outline(const double t) const { return nullptr; }
std::vector<Point> Object::points() const { return {}; }
void PathDeleter::operator()(Path *path) { delete path; }

}  // namespace omm
