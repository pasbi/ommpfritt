#include "objects/object.h"

#include <QObject>
#include <algorithm>
#include <cassert>
#include <functional>
#include <map>

#include "common.h"
#include "logging.h"
#include "objects/path.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"
#include "properties/integerproperty.h"
#include "properties/integervectorproperty.h"
#include "properties/optionproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "renderers/style.h"
#include "scene/contextes.h"
#include "scene/mailbox.h"
#include "scene/objecttree.h"
#include "scene/scene.h"
#include "serializers/jsonserializer.h"
#include "tags/styletag.h"
#include "tags/tag.h"

namespace
{
static constexpr auto almost_one = 0.9999999;
static constexpr auto CHILDREN_POINTER = "children";
static constexpr auto TAGS_POINTER = "tags";
static constexpr auto TYPE_POINTER = "type";

QPen make_bounding_box_pen()
{
  QPen pen;
  pen.setCosmetic(true);
  pen.setColor(Qt::black);
  pen.setWidth(1.0);
  return pen;
}

double length(const Geom::Curve& curve)
{
  return curve.length();
}

double length(const Geom::Path& path)
{
  return std::accumulate(path.begin(), path.end(), 0.0, [](double l, const Geom::Curve& curve) {
    return l + curve.length();
  });
}

template<typename Geometry>
std::pair<std::size_t, double> factor_time_by_distance(const Geometry& geom, double t)
{
  if (const auto n = geom.size(); n == 0) {
    return {0, 0.0};
  } else {
    std::vector<double> accumulated_lengths;
    accumulated_lengths.reserve(n + 1);
    double accu = 0.0;
    accumulated_lengths.push_back(accu);
    for (auto it = geom.begin(); it != geom.end(); ++it) {
      accu += length(*it);
      accumulated_lengths.push_back(accu);
    }

    t *= accumulated_lengths.back();
    const auto segment_end = std::find_if(accumulated_lengths.begin(),
                                          accumulated_lengths.end(),
                                          [t](double l) { return l > t; });
    if (segment_end == accumulated_lengths.begin()) {
      return {0, 0.0};
    } else {
      const auto segment_begin = std::next(segment_end, -1);
      t -= *segment_begin;
      t /= *segment_end - *segment_begin;
      return {std::distance(accumulated_lengths.begin(), segment_begin), t};
    }
  }
}

}  // namespace

namespace omm
{
QPen Object::m_bounding_box_pen = make_bounding_box_pen();
QBrush Object::m_bounding_box_brush = Qt::NoBrush;

Object::Object(Scene* scene)
    : PropertyOwner(scene), painter_path(*this), geom_paths(*this), tags(*this)
{
  static const auto category = QObject::tr("basic");
  create_property<OptionProperty>(VIEWPORT_VISIBILITY_PROPERTY_KEY, 0)
      .set_options({QObject::tr("default"), QObject::tr("hidden"), QObject::tr("visible")})
      .set_label(QObject::tr("visibility (viewport)"))
      .set_category(category);

  create_property<OptionProperty>(VISIBILITY_PROPERTY_KEY, 0)
      .set_options({QObject::tr("default"), QObject::tr("hidden"), QObject::tr("visible")})
      .set_label(QObject::tr("visibility"))
      .set_category(category);

  create_property<BoolProperty>(IS_ACTIVE_PROPERTY_KEY, true)
      .set_label(QObject::tr("active"))
      .set_category(category);

  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>"))
      .set_label(QObject::tr("Name"))
      .set_category(category);

  create_property<FloatVectorProperty>(POSITION_PROPERTY_KEY, Vec2f(0.0, 0.0))
      .set_label(QObject::tr("pos"))
      .set_category(category);

  create_property<FloatVectorProperty>(SCALE_PROPERTY_KEY, Vec2f(1.0, 1.0))
      .set_step(Vec2f(0.1, 0.1))
      .set_label(QObject::tr("scale"))
      .set_category(category);

  create_property<FloatProperty>(ROTATION_PROPERTY_KEY, 0.0)
      .set_multiplier(M_180_PI)
      .set_label(QObject::tr("rotation"))
      .set_category(category);

  create_property<FloatProperty>(SHEAR_PROPERTY_KEY, 0.0)
      .set_step(0.01)
      .set_label(QObject::tr("shear"))
      .set_category(category);
}

Object::Object(const Object& other)
    : PropertyOwner(other), TreeElement(other), painter_path(*this), geom_paths(*this),
      tags(other.tags, *this), m_draw_children(other.m_draw_children),
      m_object_tree(other.m_object_tree)
{
  for (Tag* tag : tags.items()) {
    tag->owner = this;
  }
}

Object::~Object()
{
  if (const Scene* scene = this->scene(); scene != nullptr) {
    // the object must not be selected when it gets deleted.
    // Bad things will happen otherwise.
    assert(!::contains(scene->selection(), this));
  }
}

ObjectTransformation Object::transformation() const
{
  return ObjectTransformation(property(POSITION_PROPERTY_KEY)->value<Vec2f>(),
                              property(SCALE_PROPERTY_KEY)->value<Vec2f>(),
                              property(ROTATION_PROPERTY_KEY)->value<double>(),
                              property(SHEAR_PROPERTY_KEY)->value<double>());
}

ObjectTransformation Object::global_transformation(Space space) const
{
  if (m_virtual_parent != nullptr) {
    return m_virtual_parent->global_transformation(space).apply(transformation());
  } else if (is_root()) {
    return transformation();
  } else if (space == Space::Scene && tree_parent().is_root()) {
    return transformation();
  } else {
    // TODO caching could gain some speed
    //  invalidate cache if local transformation is set or parent changes
    return tree_parent().global_transformation(space).apply(transformation());
  }
}

void Object::set_transformation(const ObjectTransformation& transformation)
{
  property(POSITION_PROPERTY_KEY)->set(transformation.translation());
  property(SCALE_PROPERTY_KEY)->set(transformation.scaling());
  property(ROTATION_PROPERTY_KEY)->set(transformation.rotation());
  property(SHEAR_PROPERTY_KEY)->set(transformation.shearing());
}

void Object::set_global_transformation(const ObjectTransformation& global_transformation,
                                       Space space)
{
  ObjectTransformation local_transformation;
  if (is_root() || (space == Space::Scene && tree_parent().is_root())) {
    local_transformation = global_transformation;
  } else {
    try {
      local_transformation
          = tree_parent().global_transformation(space).inverted().apply(global_transformation);
    } catch (const std::runtime_error&) {
      assert(false);
    }
  }
  set_transformation(local_transformation);
}

void Object::set_global_axis_transformation(const ObjectTransformation& global_transformation,
                                            Space space)
{
  const auto get_glob_trans = [space](const auto* c) { return c->global_transformation(space); };
  const auto child_transformations
      = ::transform<ObjectTransformation>(tree_children(), get_glob_trans);
  set_global_transformation(global_transformation, space);
  for (std::size_t i = 0; i < child_transformations.size(); ++i) {
    tree_children()[i]->set_global_transformation(child_transformations[i], space);
  }
}

bool Object::is_transformation_property(const Property& property) const
{
  const std::set<QString> transformation_property_keys{
      POSITION_PROPERTY_KEY,
      SCALE_PROPERTY_KEY,
      ROTATION_PROPERTY_KEY,
      SHEAR_PROPERTY_KEY,
  };
  return pmatch(&property, transformation_property_keys);
}

void Object::transform(const ObjectTransformation& transformation)
{
  set_transformation(transformation.apply(this->transformation()));
}

void Object::set_virtual_parent(const Object* parent)
{
  m_virtual_parent = parent;
}

std::ostream& operator<<(std::ostream& ostream, const Object& object)
{
  ostream << object.type() << "[" << object.name() << "]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Object* object)
{
  if (object != nullptr) {
    ostream << *object;
  } else {
    ostream << "null-Object";
  }
  return ostream;
}

void Object::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);

  const auto children_pointer = make_pointer(root, CHILDREN_POINTER);
  serializer.start_array(n_children(), children_pointer);
  for (size_t i = 0; i < n_children(); ++i) {
    const auto& child = this->tree_child(i);
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
      auto child = Object::make(child_type, static_cast<Scene*>(scene()));
      child->set_object_tree(scene()->object_tree());
      child->deserialize(deserializer, child_pointer);

      // TODO adopt sets the global transformation which is reverted by setting the local
      //  transformation immediately afterwards. That can be optimized.
      const auto t = child->transformation();
      adopt(std::move(child)).set_transformation(t);
    } catch (std::out_of_range&) {
      const auto message = QObject::tr("Failed to retrieve object type '%1'.").arg(child_type);
      LERROR << message;
      throw AbstractDeserializer::DeserializeError(message.toStdString());
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

void Object::draw_recursive(Painter& renderer, Painter::Options options) const
{
  renderer.push_transformation(transformation());
  const bool is_enabled = !!(renderer.category_filter & Painter::Category::Objects);
  if (is_enabled && is_visible(options.device_is_viewport)) {
    // TODO options.styles is overriden before being used. Why not use a local variable instead?
    // Remove the styles field from Painter::Options
    options.styles = find_styles();
    for (auto* style : options.styles) {
      draw_object(renderer, *style, options);
    }
    if (options.styles.size() == 0) {
      draw_object(renderer, *options.default_style, options);
    }

    if (!!(renderer.category_filter & Painter::Category::BoundingBox)) {
      renderer.painter->save();
      renderer.painter->setBrush(Qt::NoBrush);
      renderer.painter->setPen(m_bounding_box_pen);
      renderer.painter->drawRect(bounding_box(ObjectTransformation()));
      renderer.painter->restore();
    }

    if (!!(renderer.category_filter & Painter::Category::Handles)) {
      draw_handles(renderer);
    }
  }

  if (m_draw_children) {
    for (const auto& child : tree_children()) {
      child->draw_recursive(renderer, options);
    }
  }
  renderer.pop_transformation();
}

BoundingBox Object::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active()) {
    return BoundingBox((painter_path() * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

BoundingBox Object::recursive_bounding_box(const ObjectTransformation& transformation) const
{
  BoundingBox bounding_box;
  if (is_active()) {
    bounding_box = this->bounding_box(transformation);
  }
  for (const auto& child : tree_children()) {
    bounding_box |= child->recursive_bounding_box(transformation.apply(child->transformation()));
  }
  return bounding_box;
}

std::unique_ptr<Object> Object::repudiate(Object& repudiatee)
{
  const auto global_transformation = repudiatee.global_transformation(Space::Scene);
  auto o = TreeElement<Object>::repudiate(repudiatee);
  repudiatee.set_global_transformation(global_transformation, Space::Scene);
  return o;
}

Object& Object::adopt(std::unique_ptr<Object> adoptee, const size_t pos)
{
  const auto global_transformation = adoptee->global_transformation(Space::Scene);
  Object& o = TreeElement<Object>::adopt(std::move(adoptee), pos);
  o.set_global_transformation(global_transformation, Space::Scene);
  return o;
}

Object::ConvertedObject Object::convert() const
{
  auto converted = std::make_unique<Path>(scene());
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);
  converted->property(Path::IS_CLOSED_PROPERTY_KEY)->set(is_closed());
  converted->set(geom_paths());
  converted->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
  return {std::unique_ptr<Object>(converted.release()), true};
}

Flag Object::flags() const
{
  return Flag::Convertible;
}

void Object::copy_tags(Object& other) const
{
  for (const Tag* tag : tags.ordered_items()) {
    ListOwningContext<Tag> context(tag->clone(other), other.tags);
    other.tags.insert(context);
  }
}

void Object::on_property_value_changed(Property* property)
{
  const auto object_tree_data_changed = [this](int column) {
    if (m_object_tree != nullptr) {
      const auto index = m_object_tree->index_of(*this).siblingAtColumn(column);
      Q_EMIT m_object_tree->dataChanged(index, index);
    }
  };

  if (property == this->property(POSITION_PROPERTY_KEY)
      || property == this->property(ROTATION_PROPERTY_KEY)
      || property == this->property(SHEAR_PROPERTY_KEY)
      || property == this->property(SCALE_PROPERTY_KEY)) {
    Q_EMIT scene()->mail_box().transformation_changed(*this);
  } else if (property == this->property(IS_ACTIVE_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
    for (Object* c : all_descendants()) {
      c->m_visibility_cache_is_dirty = true;
    }
    update();
  } else if (property == this->property(NAME_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::OBJECT_COLUMN);
  } else if (property == this->property(VIEWPORT_VISIBILITY_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
    if (is_root()) {
      Q_EMIT scene()->mail_box().scene_appearance_changed();
    } else {
      Q_EMIT scene()->mail_box().object_appearance_changed(tree_parent());
    }
  } else if (property == this->property(VISIBILITY_PROPERTY_KEY)) {
    object_tree_data_changed(ObjectTree::VISIBILITY_COLUMN);
  }
}

void Object::post_create_hook()
{
}

void Object::update()
{
  painter_path.invalidate();
  geom_paths.invalidate();
  if (Scene* scene = this->scene(); scene != nullptr) {
    Q_EMIT scene->mail_box().object_appearance_changed(*this);
  }
}

double Object::apply_border(double t, Border border)
{
  switch (border) {
  case Border::Clamp:
    return std::min(1.0, std::max(0.0, t));
  case Border::Wrap:
    return fmod(fmod(t, 1.0) + 1.0, 1.0);
  case Border::Hide:
    return (t >= 0.0 && t <= 1.0) ? t : -1.0;
  case Border::Reflect: {
    const bool flip = int(t / 1.0) % 2 == 1;
    t = apply_border(t, Border::Wrap);
    return flip ? (1.0 - t) : t;
  }
  }
  Q_UNREACHABLE();
}

void Object::set_oriented_position(const Point& op, const bool align)
{
  auto transformation = global_transformation(Space::Scene);
  if (align) {
    transformation.set_rotation(op.rotation());
  }
  transformation.set_translation(op.position);
  set_global_transformation(transformation, Space::Scene);
}

bool Object::is_active() const
{
  return property(IS_ACTIVE_PROPERTY_KEY)->value<bool>();
}

bool Object::is_visible(bool viewport) const
{
  const QString key = viewport ? VIEWPORT_VISIBILITY_PROPERTY_KEY : VISIBILITY_PROPERTY_KEY;
  const auto compute_visibility = [this, key, viewport]() {
    switch (property(key)->value<Visibility>()) {
    case Visibility::Hidden:
      return false;
    case Visibility::Visible:
      return true;
    default:
      if (is_root()) {
        return true;
      } else {
        return tree_parent().is_visible(viewport);
      }
    }
  };

  if (m_visibility_cache_is_dirty) {
    m_visibility_cache_value = compute_visibility();
  }
  return m_visibility_cache_value;
}

std::vector<const omm::Style*> Object::find_styles() const
{
  const auto get_style = [](const omm::Tag* tag) -> const omm::Style* {
    if (tag->type() == omm::StyleTag::TYPE) {
      const auto* property_owner = tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)
                                       ->value<omm::ReferenceProperty::value_type>();
      assert(property_owner == nullptr || property_owner->kind == omm::Kind::Style);
      return static_cast<const omm::Style*>(property_owner);
    } else {
      return nullptr;
    }
  };

  const auto tags = this->tags.ordered_items();
  return ::filter_if(::transform<const omm::Style*>(tags, get_style), ::is_not_null);
}

Point Object::pos(const Geom::PathVectorTime& t) const
{
  auto&& paths = geom_paths();
  if (const auto n = paths.curveCount(); n == 0) {
    return Point();
  } else if (t.path_index >= paths.size()) {
    return Point();
  } else if (auto&& path = paths[t.path_index]; t.curve_index >= path.size()) {
    return Point();
  } else {
    auto&& curve = path[t.curve_index];

    // the tangent behaves strange if s is very close to
    // 1.0 and the curve is the last one in the path.
    const double s = std::clamp(t.t, 0.0, almost_one);
    const auto tangent = curve.unitTangentAt(s);
    auto position = curve.pointAt(s);
    const auto convert = [](const Geom::Point& p) { return Vec2{p.x(), p.y()}; };
    return Point(convert(position),
                 PolarCoordinates(convert(tangent)),
                 PolarCoordinates(-convert(tangent)));
  }
}

bool Object::is_closed() const
{
  return false;
}

bool Object::contains(const Vec2f& point) const
{
  const auto path_vector = geom_paths();
  const auto winding = path_vector.winding(Geom::Point{point.x, point.y});
  return std::abs(winding) % 2 == 1;
}

Geom::PathVector Object::paths() const
{
  return Geom::PathVector();
}

Geom::PathVectorTime Object::compute_path_vector_time(double t, Interpolation interpolation) const
{
  t = std::clamp(t, 0.0, almost_one);
  const auto path_vector = paths();
  if (path_vector.empty()) {
    return Geom::PathVectorTime(0, 0, 0.0);
  }

  switch (interpolation) {
  case Interpolation::Natural: {
    double path_index;
    const double path_position = std::modf(t * path_vector.size(), &path_index);
    return compute_path_vector_time(path_index, path_position, interpolation);
  }
  case Interpolation::Distance: {
    const auto [i, tp] = factor_time_by_distance(path_vector, t);
    return compute_path_vector_time(i, tp, interpolation);
  }
  default:
    return {};
  }
}

Geom::PathVectorTime
Object::compute_path_vector_time(int path_index, double t, Interpolation interpolation) const
{
  if (path_index < 0) {
    return compute_path_vector_time(t, interpolation);
  }

  t = std::clamp(t, 0.0, almost_one);
  const auto path_vector = geom_paths();
  if (static_cast<std::size_t>(path_index) >= path_vector.size()) {
    return Geom::PathVectorTime(path_index, 0, 0.0);
  }

  const auto path = path_vector[path_index];
  if (path.empty()) {
    return Geom::PathVectorTime(path_index, 0, 0.0);
  }

  switch (interpolation) {
  case Interpolation::Natural: {
    double curve_index;
    const double curve_position = std::modf(t * path.size(), &curve_index);
    return Geom::PathVectorTime(path_index, curve_index, curve_position);
  }
  case Interpolation::Distance: {
    const auto [i, tc] = factor_time_by_distance(path, t);
    return Geom::PathVectorTime(path_index, i, tc);
  }
  default:
    return {};
  }
}

void Object::update_recursive()
{
  // it's important to first update the children because of the way e.g. Cloner does its caching.
  for (auto* child : tree_children()) {
    child->update_recursive();
  }
  update();
  //  Q_EMIT scene()->mail_box.appearance_changed(*this);
}

QString Object::tree_path() const
{
  const QString path = is_root() ? "" : tree_parent().tree_path();
  return path + "/" + name();
}

void Object::draw_object(Painter& renderer, const Style& style, Painter::Options options) const
{
  if (QPainter* painter = renderer.painter; painter != nullptr && is_active()) {
    if (const auto painter_path = this->painter_path(); !painter_path.isEmpty()) {
      renderer.set_style(style, *this, options);
      if (!is_closed()) {
        renderer.painter->setBrush(Qt::NoBrush);
      }
      painter->drawPath(painter_path);
      const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
      const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();

      const auto paths = this->geom_paths();
      for (std::size_t path_index = 0; path_index < paths.size(); ++path_index) {
        const auto pos = [this, path_index](const double t) {
          const auto tt = compute_path_vector_time(path_index, t);
          return this->pos(tt).rotated(M_PI_2);
        };
        style.start_marker.draw_marker(renderer, pos(0.0), marker_color, width);
        style.end_marker.draw_marker(renderer, pos(1.0), marker_color, width);
      }
    }
  }
}

void Object::draw_handles(Painter&) const
{
}

void Object::set_object_tree(ObjectTree& object_tree)
{
  if (m_object_tree == nullptr) {
    m_object_tree = &object_tree;
  } else {
    assert(m_object_tree == &object_tree);
  }
}

void Object::on_child_added(Object& child)
{
  TreeElement::on_child_added(child);
  Q_EMIT scene()->mail_box().object_appearance_changed(*this);
}

void Object::on_child_removed(Object& child)
{
  TreeElement::on_child_removed(child);
  Q_EMIT scene()->mail_box().object_appearance_changed(*this);
}

void Object::listen_to_changes(const std::function<Object*()>& get_watched)
{
  connect(&scene()->mail_box(),
          &MailBox::object_appearance_changed,
          this,
          [get_watched, this](Object& o) {
            Object* r = get_watched();
            if (r != nullptr) {
              if (r->is_ancestor_of(*this)) {
                {
                  QSignalBlocker blocker(&scene()->mail_box());
                  update();
                }
                Q_EMIT scene()->mail_box().scene_appearance_changed();
              } else if (r->is_ancestor_of(o)) {
                update();
              }
            }
          });
}

void Object::listen_to_children_changes()
{
  const auto on_change = [this](Object& o) {
    if (&o != this && is_ancestor_of(o)) {
      update();
    }
  };
  connect(&scene()->mail_box(), &MailBox::transformation_changed, this, on_change);
  connect(&scene()->mail_box(), &MailBox::object_appearance_changed, this, on_change);
}

Path::Segment Object::path_to_segment(const Geom::Path& path, bool is_closed)
{
  const auto to_vec = [](const Geom::Point& p) -> Vec2f { return {p.x(), p.y()}; };
  Path::Segment segment;
  segment.reserve(path.size_default() + 1);
  const auto n = path.size();
  for (std::size_t i = 0; i < n; ++i) {
    const auto& c = dynamic_cast<const Geom::CubicBezier&>(path[i]);
    const auto p0 = to_vec(c[0]);
    if (segment.empty()) {
      segment.push_back(Point(p0));
    }
    segment.back().right_tangent = PolarCoordinates(to_vec(c[1]) - p0);
    const auto p1 = to_vec(c[3]);
    auto& pref = [wrap = is_closed && i == n - 1, p1, &segment]() -> decltype(auto) {
      if (wrap) {
        return segment.front();
      } else {
        segment.push_back(Point(p1));
        return segment.back();
      }
    }();
    pref.left_tangent = PolarCoordinates(to_vec(c[2]) - p1);
  }
  if (is_closed) {
    assert(path.size() == segment.size());
  } else {
    // path counts number of curves, segments counts number of points
    assert(path.size() + 1 == segment.size());
  }
  return segment;
}

Geom::Path Object::segment_to_path(Segment segment, bool is_closed, InterpolationMode interpolation)
{
  const auto pts = [](const std::array<Vec2f, 4>& pts) {
    return ::transform<Geom::Point, std::vector>(pts, [](const auto& p) {
      return Geom::Point(p.x, p.y);
    });
  };

  std::vector<Geom::CubicBezier> bzs;
  const std::size_t n = segment.size();
  const std::size_t m = is_closed ? n : n - 1;
  bzs.reserve(m);

  if (interpolation == InterpolationMode::Smooth) {
    segment = [&segment, is_closed]() {
      Segment new_segment;
      new_segment.reserve(segment.size());
      for (std::size_t i = 0; i < segment.size(); ++i) {
        new_segment.push_back(Path::smoothen_point(segment, is_closed, i));
      }
      return new_segment;
    }();
  }

  for (std::size_t i = 0; i < m; ++i) {
    const std::size_t j = (i + 1) % n;
    switch (interpolation) {
    case InterpolationMode::Bezier:
      [[fallthrough]];
    case InterpolationMode::Smooth:
      bzs.emplace_back(pts({segment[i].position,
                            segment[i].right_position(),
                            segment[j].left_position(),
                            segment[j].position}));
      break;
    case InterpolationMode::Linear:
      bzs.emplace_back(pts({segment[i].position,
                            (2.0 * segment[i].position + 1.0 * segment[j].position) / 3.0,
                            (1.0 * segment[i].position + 2.0 * segment[j].position) / 3.0,
                            segment[j].position}));
      break;
    }
  }
  return Geom::Path(bzs.begin(), bzs.end(), is_closed);
}

QPainterPath Object::CachedQPainterPathGetter::compute() const
{
  static const auto qpoint = [](const Geom::Point& point) { return QPointF{point[0], point[1]}; };
  const auto path_vector = m_self.paths();
  QPainterPath pp;
  for (const Geom::Path& path : path_vector) {
    pp.moveTo(qpoint(path.initialPoint()));
    for (const Geom::Curve& curve : path) {
      const auto& cbc = dynamic_cast<const Geom::CubicBezier&>(curve);
      pp.cubicTo(qpoint(cbc[1]), qpoint(cbc[2]), qpoint(cbc[3]));
    }
  }
  return pp;
}

Geom::PathVector Object::CachedGeomPathVectorGetter::compute() const
{
  return m_self.paths();
}

}  // namespace omm
