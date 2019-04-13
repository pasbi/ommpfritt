#include "objects/cloner.h"

#include <QObject>

#include "properties/integerproperty.h"
#include "properties/stringproperty.h"
#include "properties/optionsproperty.h"
#include "properties/floatproperty.h"
#include "properties/vectorproperty.h"
#include "properties/referenceproperty.h"
#include "properties/boolproperty.h"
#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/pythonengine.h"
#include "objects/empty.h"

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

Cloner::Cloner(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("Cloner").toStdString();
  auto& mode_property = add_property<OptionsProperty>(MODE_PROPERTY_KEY);
  mode_property.set_options({ QObject::tr("Linear").toStdString(),
    QObject::tr("Grid").toStdString(), QObject::tr("Radial").toStdString(),
    QObject::tr("Path").toStdString(), QObject::tr("Script").toStdString(),
    QObject::tr("Fill Random").toStdString() })
    .set_label(QObject::tr("mode").toStdString())
    .set_category(category);

  add_property<IntegerProperty>(COUNT_PROPERTY_KEY, 3)
    .set_range(0, max)
    .set_label(QObject::tr("count").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Linear, Mode::Radial, Mode::Path,
                                              Mode::Script, Mode::FillRandom });

  add_property<IntegerVectorProperty>(COUNT_2D_PROPERTY_KEY, Vec2i(3, 3))
    .set_range(Vec2i(0, 0), Vec2i(max, max))
    .set_label(QObject::tr("count").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Grid });

  add_property<FloatVectorProperty>(DISTANCE_2D_PROPERTY_KEY, Vec2f(10.0, 10.0))
    .set_step(Vec2f(0.1, 0.1))
    .set_label(QObject::tr("distance").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Linear, Mode::Grid });

  add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 50.0)
    .set_step(0.1)
    .set_label(QObject::tr("radius").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Radial });

  add_property<ReferenceProperty>(PATH_REFERENCE_PROPERTY_KEY)
    .set_allowed_kinds(Kind::Object)
    .set_required_flags(Flag::IsPathLike)
    .set_label(QObject::tr("path").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Path, Mode::FillRandom });

  add_property<FloatProperty>(START_PROPERTY_KEY, 0.0)
    .set_step(0.01)
    .set_label(QObject::tr("start").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Radial, Mode::Path });

  add_property<FloatProperty>(END_PROPERTY_KEY, 1.0)
    .set_step(0.01)
    .set_label(QObject::tr("end").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Radial, Mode::Path });

  add_property<BoolProperty>(ALIGN_PROPERTY_KEY, true)
    .set_label(QObject::tr("align").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Radial, Mode::Path });

  add_property<OptionsProperty>(BORDER_PROPERTY_KEY)
    .set_options( { QObject::tr("Clamp").toStdString(), QObject::tr("Wrap").toStdString(),
      QObject::tr("Hide").toStdString(), QObject::tr("Reflect").toStdString() } )
    .set_label(QObject::tr("border").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Radial, Mode::Path });

  add_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
    .set_mode(StringProperty::Mode::Code)
    .set_label(QObject::tr("code").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::Script });

  add_property<IntegerProperty>(SEED_PROPERTY_KEY, 12345)
    .set_label(QObject::tr("seed").toStdString())
    .set_category(category)
    .set_enabled_buddy<Mode>(mode_property, { Mode::FillRandom });
}


void Cloner::draw_object(AbstractRenderer& renderer, const Style& style)
{
  if (is_active()) {
    assert(&renderer.scene == scene());
    for (auto&& clone : make_clones()) { clone->draw_recursive(renderer, style); }
    m_draw_children = false;
  } else {
    m_draw_children = true;
  }
}

BoundingBox Cloner::bounding_box()
{
  BoundingBox bb;
  for (auto&& clone : make_clones()) {
    bb |= clone->transformation().apply(clone->bounding_box());
  }
  return bb;
}

Cloner::Mode Cloner::mode() const { return property(MODE_PROPERTY_KEY).value<Mode>(); }

bool Cloner::contains(const Vec2f &pos)
{
  for (auto&& clone : make_clones()) {
    if (clone->contains(clone->transformation().apply_to_position(pos))) {
      return true;
    }
  }
  return false;
}

std::string Cloner::type() const { return TYPE; }
std::unique_ptr<Object> Cloner::clone() const { return std::make_unique<Cloner>(*this); }
AbstractPropertyOwner::Flag Cloner::flags() const
{
  return Object::flags() | Flag::Convertable | Flag::HasScript;
}

std::unique_ptr<Object> Cloner::convert()
{
  std::unique_ptr<Object> converted = std::make_unique<Empty>(scene());
  copy_properties(*converted);
  copy_tags(*converted);

  auto clones = make_clones();
  for (std::size_t i = 0; i < clones.size(); ++i) {
    const auto local_transformation = clones[i]->transformation();
    auto& clone = converted->adopt(std::move(clones[i]));
    const std::string name = clone.name() + " " + std::to_string(i);
    clone.property(NAME_PROPERTY_KEY).set(name);
    clone.set_transformation(local_transformation);
  }

  return converted;
}

std::vector<std::unique_ptr<Object>> Cloner::make_clones()
{
  const auto count = [this]() -> std::size_t {
    switch (mode()) {
    case Mode::Linear:
    case Mode::Radial:
    case Mode::Path:
    case Mode::Script:
    case Mode::FillRandom:
      return static_cast<std::size_t>(property(COUNT_PROPERTY_KEY).value<int>());
    case Mode::Grid: {
      const auto c = property(COUNT_2D_PROPERTY_KEY).value<Vec2i>();
      return static_cast<std::size_t>(c.x * c.y);
    }
    }
    Q_UNREACHABLE();
  };

  auto clones = copy_children(count());
  for (std::size_t i = 0; i < clones.size(); ++i) {
    switch (mode()) {
    case Mode::Linear: set_linear(*clones[i], i); break;
    case Mode::Radial: set_radial(*clones[i], i); break;
    case Mode::Path: set_path(*clones[i], i); break;
    case Mode::Script: set_by_script(*clones[i], i); break;
    case Mode::Grid: set_grid(*clones[i], i); break;
    case Mode::FillRandom: set_fillrandom(*clones[i], i); break;
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
      clones.push_back(child(i % n_children).clone());
    }
  }
  return clones;
}

double Cloner::get_t(std::size_t i, const bool inclusive) const
{
  const auto n = property(COUNT_PROPERTY_KEY).value<int>() + (inclusive ? 0 : 1);
  const auto start = property(START_PROPERTY_KEY).value<double>();
  const auto end = property(END_PROPERTY_KEY).value<double>();
  const auto border = property(BORDER_PROPERTY_KEY).value<Border>();

  if (n <= 1) {
    return 0.0;
  } else {
    const auto spacing =  (end - start) / (n-1);
    return apply_border(start + spacing * i, border);
  };
}

void Cloner::set_linear(Object& object, std::size_t i)
{
  const Vec2f pos = static_cast<double>(i) * property(DISTANCE_2D_PROPERTY_KEY).value<Vec2f>();
  auto t = object.transformation();
  t.set_translation(pos);
  object.set_transformation(t);
}

void Cloner::set_grid(Object& object, std::size_t i)
{
  const auto n = property(COUNT_2D_PROPERTY_KEY).value<Vec2i>();
  const auto v = property(DISTANCE_2D_PROPERTY_KEY).value<Vec2f>();
  auto t = object.transformation();
  t.set_translation({ v.x * (i % static_cast<ulong>(n.x)),
                      v.y * (i / static_cast<ulong>(n.x)) });
  object.set_transformation(t);
}

void Cloner::set_radial(Object& object, std::size_t i)
{
  const double angle = 2*M_PI * get_t(i, false);
  const double r = property(RADIUS_PROPERTY_KEY).value<double>();
  const Point op({std::cos(angle) * r, std::sin(angle) * r}, angle + M_PI/2.0);
  object.set_oriented_position(op, property(ALIGN_PROPERTY_KEY).value<bool>());
}

void Cloner::set_path(Object& object, std::size_t i)
{
  auto* o = property(PATH_REFERENCE_PROPERTY_KEY).value<AbstractPropertyOwner*>();

  const bool align = property(ALIGN_PROPERTY_KEY).value<bool>();
  object.set_position_on_path(o, align, get_t(i, true));
}

void Cloner::set_by_script(Object& object, std::size_t i)
{
  using namespace pybind11::literals;
  const auto locals = pybind11::dict( "id"_a=i,
                                      "count"_a=property(COUNT_PROPERTY_KEY).value<int>(),
                                      "copy"_a=ObjectWrapper::make(object),
                                      "this"_a=ObjectWrapper::make(*this),
                                      "scene"_a=SceneWrapper(*scene()) );
  scene()->python_engine.exec(property(CODE_PROPERTY_KEY).value<std::string>(), locals, this);
}

void Cloner::set_fillrandom(Object &object, std::size_t i)
{
  const auto seed = property(SEED_PROPERTY_KEY).value<int>();
}


}  // namespace omm
