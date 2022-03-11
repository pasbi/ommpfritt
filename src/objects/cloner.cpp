#include "objects/cloner.h"

#include <QObject>

#include "objects/empty.h"
#include "path/pathvector.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/floatvectorproperty.h"
#include "properties/integerproperty.h"
#include "properties/integervectorproperty.h"
#include "properties/optionproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "python/objectwrapper.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/scene.h"
#include "scene/mailbox.h"
#include <random>

namespace
{
constexpr auto default_script = R"(import math
import numpy as np
np.random.seed(id)
copy.set("position", [ id*100, 0.0 ])
copy.set("rotation", id*math.pi/10.0)
copy.set("scale", np.random.random(2)+0.5)
)";

constexpr auto max = std::numeric_limits<int>::max();

}  // namespace

namespace omm
{
class Style;

Cloner::Cloner(Scene* scene) : Object(scene), path_properties("", *this)
{
  static constexpr double STEP_01 = 0.1;
  static constexpr double STEP_001 = 0.01;
  static constexpr int DEFAULT_SEED = 12345;
  static constexpr double DEFAULT_DISTANCE = 100.0;
  static const auto category = QObject::tr("Cloner");
  auto& mode_property = create_property<OptionProperty>(MODE_PROPERTY_KEY);
  mode_property
      .set_options({QObject::tr("Linear"),
                    QObject::tr("Grid"),
                    QObject::tr("Radial"),
                    QObject::tr("Path"),
                    QObject::tr("Script"),
                    QObject::tr("Fill Random")})
      .set_label(QObject::tr("mode"))
      .set_category(category);

  create_property<IntegerProperty>(COUNT_PROPERTY_KEY, 3)
      .set_range(0, max)
      .set_label(QObject::tr("count"))
      .set_category(category);

  create_property<IntegerVectorProperty>(COUNT_2D_PROPERTY_KEY, Vec2i(3, 3))
      .set_range(Vec2i(0, 0), Vec2i(max, max))
      .set_label(QObject::tr("count"))
      .set_category(category);

  create_property<FloatVectorProperty>(DISTANCE_2D_PROPERTY_KEY,
                                       Vec2f(DEFAULT_DISTANCE, DEFAULT_DISTANCE))
      .set_step(Vec2f(STEP_01, STEP_01))
      .set_label(QObject::tr("distance"))
      .set_category(category);

  create_property<FloatProperty>(RADIUS_PROPERTY_KEY, DEFAULT_DISTANCE)
      .set_step(STEP_01)
      .set_label(QObject::tr("radius"))
      .set_category(category);

  path_properties.make_properties(category);

  create_property<FloatProperty>(START_PROPERTY_KEY, 0.0)
      .set_step(STEP_001)
      .set_label(QObject::tr("start"))
      .set_category(category);

  create_property<FloatProperty>(END_PROPERTY_KEY, 1.0)
      .set_step(STEP_001)
      .set_label(QObject::tr("end"))
      .set_category(category);

  create_property<OptionProperty>(BORDER_PROPERTY_KEY)
      .set_options(
          {QObject::tr("Clamp"), QObject::tr("Wrap"), QObject::tr("Hide"), QObject::tr("Reflect")})
      .set_label(QObject::tr("border"))
      .set_category(category);

  create_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
      .set_mode(StringProperty::Mode::Code)
      .set_label(QObject::tr("code"))
      .set_category(category);

  create_property<IntegerProperty>(SEED_PROPERTY_KEY, DEFAULT_SEED)
      .set_label(QObject::tr("seed"))
      .set_category(category);

  create_property<OptionProperty>(ANCHOR_PROPERTY_KEY, 0)
      .set_options({QObject::tr("Path"), QObject::tr("this")})
      .set_label(QObject::tr("anchor"))
      .set_category(category);

  polish();
}

Cloner::Cloner(const Cloner& other) : Object(other), path_properties("", *this)
{
  polish();
}

void Cloner::polish()
{
  update_property_visibility(property(MODE_PROPERTY_KEY)->value<Mode>());
  Cloner::update();
}

const Object* Cloner::path_object_reference() const
{
  const auto* property = this->property(PathProperties::PATH_REFERENCE_PROPERTY_KEY);
  return kind_cast<const Object*>(property->value<AbstractPropertyOwner*>());
}

void Cloner::draw_object(Painter& renderer,
                         const Style& style,
                         const PainterOptions& options) const
{
  assert(&renderer.scene == scene());
  auto options_copy = options;
  options_copy.default_style = &style;
  for (auto&& clone : m_clones) {
    clone->draw_recursive(renderer, options_copy);
  }
}

BoundingBox Cloner::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active()) {
    BoundingBox bb;
    for (auto&& clone : m_clones) {
      bb |= clone->recursive_bounding_box(transformation.apply(clone->transformation()));
    }
    return bb;
  } else {
    return BoundingBox{};
  }
}

BoundingBox Cloner::recursive_bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active()) {
    // ignore children
    return bounding_box(transformation);
  } else {
    return Object::recursive_bounding_box(transformation);
  }
}

Cloner::Mode Cloner::mode() const
{
  return property(MODE_PROPERTY_KEY)->value<Mode>();
}

bool Cloner::contains(const Vec2f& pos) const
{
  return std::any_of(m_clones.begin(), m_clones.end(), [pos](auto&& object) {
    return object->contains(object->transformation().apply_to_position(pos));
  });
}

void Cloner::update()
{
  {
    QSignalBlocker blocker(&scene()->mail_box());
    if (is_active()) {
      m_clones = make_clones();
      m_draw_children = false;
    } else {
      m_clones.clear();
      m_draw_children = true;
    }
  }
  Object::update();
}

PathVector Cloner::compute_path_vector() const
{
  return join(util::transform(m_clones, [](const auto& up) { return up.get(); }));
}

void Cloner::on_property_value_changed(Property* property)
{
  static const std::set<QString> common = {COUNT_PROPERTY_KEY,
                                           COUNT_2D_PROPERTY_KEY,
                                           DISTANCE_2D_PROPERTY_KEY,
                                           RADIUS_PROPERTY_KEY,
                                           START_PROPERTY_KEY,
                                           END_PROPERTY_KEY,
                                           BORDER_PROPERTY_KEY,
                                           CODE_PROPERTY_KEY,
                                           SEED_PROPERTY_KEY,
                                           ANCHOR_PROPERTY_KEY};

  if (pmatch(property, ::merge(common, PathProperties::keys))) {
    update();
  } else if (property == this->property(MODE_PROPERTY_KEY)) {
    update_property_visibility(property->value<Mode>());
    update();
  } else {
    if (is_transformation_property(*property)
        && this->property(ANCHOR_PROPERTY_KEY)->value<Anchor>() == Anchor::Path) {
      update();
    }
    Object::on_property_value_changed(property);
  }
}

void Cloner::on_child_added(Object& child)
{
  Object::on_child_added(child);
  update();
}

void Cloner::on_child_removed(Object& child)
{
  Object::on_child_removed(child);
  update();
}

void Cloner::update_property_visibility(Mode mode)
{
  static const std::set<QString> properties = ::merge(std::set<QString>{CODE_PROPERTY_KEY,
                                                                        COUNT_PROPERTY_KEY,
                                                                        COUNT_2D_PROPERTY_KEY,
                                                                        DISTANCE_2D_PROPERTY_KEY,
                                                                        RADIUS_PROPERTY_KEY,
                                                                        START_PROPERTY_KEY,
                                                                        END_PROPERTY_KEY,
                                                                        BORDER_PROPERTY_KEY,
                                                                        SEED_PROPERTY_KEY,
                                                                        ANCHOR_PROPERTY_KEY},
                                                      PathProperties::keys);
  static const std::map<Mode, std::set<QString>> visibility_map{
      {Mode::Linear, {COUNT_PROPERTY_KEY, DISTANCE_2D_PROPERTY_KEY}},
      {Mode::Radial,
       {COUNT_PROPERTY_KEY,
        RADIUS_PROPERTY_KEY,
        START_PROPERTY_KEY,
        END_PROPERTY_KEY,
        PathProperties::ALIGN_PROPERTY_KEY,
        BORDER_PROPERTY_KEY}},
      {Mode::Path,
       ::merge(std::set<QString>{COUNT_PROPERTY_KEY,
                                 START_PROPERTY_KEY,
                                 END_PROPERTY_KEY,
                                 PathProperties::ALIGN_PROPERTY_KEY,
                                 BORDER_PROPERTY_KEY,
                                 ANCHOR_PROPERTY_KEY},
               PathProperties::keys)},
      {Mode::Script, {COUNT_PROPERTY_KEY, CODE_PROPERTY_KEY}},
      {Mode::FillRandom,
       {COUNT_PROPERTY_KEY,
        PathProperties::PATH_REFERENCE_PROPERTY_KEY,
        SEED_PROPERTY_KEY,
        ANCHOR_PROPERTY_KEY}},
      {Mode::Grid, {COUNT_2D_PROPERTY_KEY, DISTANCE_2D_PROPERTY_KEY}}};

  for (const QString& pk : properties) {
    if (properties.contains(pk)) {
      property(pk)->set_visible(visibility_map.at(mode).contains(pk));
    }
  }
  Q_EMIT property_visibility_changed();
}

QString Cloner::type() const
{
  return TYPE;
}
Flag Cloner::flags() const
{
  return Object::flags() | Flag::HasScript;
}

std::unique_ptr<Object> Cloner::convert(bool& keep_children) const
{
  std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);

  for (std::size_t i = 0; i < m_clones.size(); ++i) {
    const auto local_transformation = m_clones[i]->transformation();
    auto& clone = converted->adopt(m_clones[i]->clone());
    const QString name = clone.name() + QString(" %1").arg(i);
    clone.property(NAME_PROPERTY_KEY)->set(name);
    clone.set_transformation(local_transformation);
  }

  keep_children = !is_active();
  return converted;
}

std::vector<std::unique_ptr<Object>> Cloner::make_clones()
{
  const auto count = [this]() -> std::size_t {
    switch (mode()) {
    case Mode::Linear:  // NOLINT(bugprone-branch-clone)
      [[fallthrough]];
    case Mode::Radial:
      [[fallthrough]];
    case Mode::Path:
      [[fallthrough]];
    case Mode::Script:
      [[fallthrough]];
    case Mode::FillRandom:
      return static_cast<std::size_t>(property(COUNT_PROPERTY_KEY)->value<int>());
    case Mode::Grid: {
      const auto c = property(COUNT_2D_PROPERTY_KEY)->value<Vec2i>();
      return static_cast<std::size_t>(std::max(0, c.x))
             * static_cast<std::size_t>(std::max(0, c.y));
    }
    }
    Q_UNREACHABLE();
  };

  const auto seed = property(SEED_PROPERTY_KEY)->value<int>();
  std::random_device dev;
  std::mt19937 rng(dev());
  rng.seed(static_cast<decltype(rng)::result_type>(seed));

  auto clones = copy_children(count());
  for (std::size_t i = 0; i < clones.size(); ++i) {
    switch (mode()) {
    case Mode::Linear:
      set_linear(*clones[i], i);
      break;
    case Mode::Radial:
      set_radial(*clones[i], i);
      break;
    case Mode::Path:
      set_path(*clones[i], i);
      break;
    case Mode::Script:
      set_by_script(*clones[i], i);
      break;
    case Mode::Grid:
      set_grid(*clones[i], i);
      break;
    case Mode::FillRandom:
      set_fillrandom(*clones[i], rng);
      break;
    }
  }

  return clones;
}

std::vector<std::unique_ptr<Object>> Cloner::copy_children(const std::size_t count)
{
  const auto n_children = this->n_children();
  std::vector<std::unique_ptr<Object>> clones;
  if (n_children > 0 && count > 0) {
    clones.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
      auto clone = tree_child(i % n_children).clone();
      clone->set_virtual_parent(this);
      clone->update();
      clones.push_back(std::move(clone));
    }
  }
  return clones;
}

double Cloner::get_t(std::size_t i) const
{
  const auto n = property(COUNT_PROPERTY_KEY)->value<int>() + 1;
  const auto start = property(START_PROPERTY_KEY)->value<double>();
  const auto end = property(END_PROPERTY_KEY)->value<double>();
  const auto border = property(BORDER_PROPERTY_KEY)->value<Border>();

  if (n <= 1) {
    return 0.0;
  } else {
    const auto spacing = (end - start) / (n - 1);
    return apply_border(start + spacing * static_cast<double>(i), border);
  }
}

void Cloner::set_linear(Object& object, std::size_t i)
{
  const Vec2f pos = static_cast<double>(i) * property(DISTANCE_2D_PROPERTY_KEY)->value<Vec2f>();
  auto t = object.transformation();
  t.set_translation(pos);
  object.set_transformation(t);
}

void Cloner::set_grid(Object& object, std::size_t i)
{
  const auto n = property(COUNT_2D_PROPERTY_KEY)->value<Vec2i>();
  const auto v = property(DISTANCE_2D_PROPERTY_KEY)->value<Vec2f>();
  auto t = object.transformation();
  const auto [q, r] = std::div(static_cast<int>(i), static_cast<int>(n.x));
  t.set_translation({v.x * r, v.y * q});
  object.set_transformation(t);
}

void Cloner::set_radial(Object& object, std::size_t i)
{
  const double angle = 2 * M_PI * get_t(i);
  const double r = property(RADIUS_PROPERTY_KEY)->value<double>();
  const Point op({std::cos(angle) * r, std::sin(angle) * r}, angle + M_PI / 2.0);
  object.set_oriented_position(op, property(PathProperties::ALIGN_PROPERTY_KEY)->value<bool>());
}

void Cloner::set_path(Object& object, std::size_t i)
{
  if (const auto* const o = path_object_reference(); o == nullptr) {
    return;
  } else {
    const double t = get_t(i);
    const auto transformation = (property(ANCHOR_PROPERTY_KEY)->value<Anchor>() == Anchor::Path)
                                    ? o->global_transformation(Space::Scene)
                                          .apply(global_transformation(Space::Scene).inverted())
                                    : ObjectTransformation();
    path_properties.apply_transformation(object, t, transformation);
  }
}

void Cloner::set_by_script(Object& object, std::size_t i)
{
  using namespace pybind11::literals;
  auto locals = pybind11::dict("id"_a = i,
                               "count"_a = property(COUNT_PROPERTY_KEY)->value<int>(),
                               "copy"_a = ObjectWrapper::make(object),
                               "this"_a = ObjectWrapper::make(*this),
                               "scene"_a = SceneWrapper(*scene()));
  PythonEngine::instance().exec(property(CODE_PROPERTY_KEY)->value<QString>(), locals, this);
}

void Cloner::set_fillrandom(Object& object, std::mt19937& rng)
{
  if (const auto* const o = path_object_reference(); o != nullptr) {
    auto position = [&rng, o]() {
      static constexpr auto max_rejections = 1000;
      auto dist = std::uniform_real_distribution<double>(0, 1);
      const BoundingBox bb = o->bounding_box(ObjectTransformation());
      for (std::size_t i = 0; i < max_rejections; ++i) {
        const Vec2f p(dist(rng) * bb.width() + bb.left(), dist(rng) * bb.height() + bb.top());
        if (o->contains(p)) {
          return p;
        }
      }

      LWARNING << "Giving up to create sample within path after " << max_rejections
               << " rejections.";
      LINFO << "Return a random point on edge instead.";

      const auto t = dist(rng);
      return o->pos(o->compute_path_vector_time(t)).position();
    }();

    if (property(ANCHOR_PROPERTY_KEY)->value<Anchor>() == Anchor::Path) {
      const auto gti = global_transformation(Space::Scene).inverted();
      position = o->global_transformation(Space::Scene).apply_to_position(position);
      position = gti.apply_to_position(position);
    }

    auto t = object.transformation();
    t.set_translation(position);
    object.set_transformation(t);
  }
}

}  // namespace omm
