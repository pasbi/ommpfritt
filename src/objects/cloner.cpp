#include "objects/cloner.h"

#include <QObject>

#include "properties/integerproperty.h"
#include "properties/stringproperty.h"
#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/pythonengine.h"
#include "objects/empty.h"

namespace omm
{

class Style;

Cloner::Cloner(Scene* scene) : Object(scene)
{
  m_draw_children = false;
  add_property<IntegerProperty>(COUNT_PROPERTY_KEY)
    .set_label(QObject::tr("count").toStdString())
    .set_category(QObject::tr("Cloner").toStdString());
  add_property<StringProperty>(CODE_PROPERTY_KEY, "")
    .set_is_multi_line(true)
    .set_label(QObject::tr("code").toStdString())
    .set_category(QObject::tr("Cloner").toStdString());
}

void Cloner::render(AbstractRenderer& renderer, const Style& style)
{
  assert(&renderer.scene == scene());
  for (auto&& clone : make_clones()) {
    renderer.push_transformation(clone->transformation());
    clone->render_recursive(renderer, style);
    renderer.pop_transformation();
  }
}

std::vector<std::unique_ptr<Object>> Cloner::make_clones()
{
  using namespace pybind11::literals;
  const auto n_children = this->n_children();
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();


  std::vector<std::unique_ptr<Object>> clones;
  if (n_children > 0) {
    clones.reserve(count);
    for (int i = 0; i < count; ++i) {
      clones.push_back(child(i % n_children).clone());
    }
    for (int i = 0; i < count; ++i) {
      auto& copy = *clones.at(i % clones.size());
      const auto locals = pybind11::dict( "id"_a=i,
                                          "count"_a=count,
                                          "copy"_a=ObjectWrapper::make(copy),
                                          "this"_a=ObjectWrapper::make(*this),
                                          "scene"_a=SceneWrapper(*scene()) );
      scene()->python_engine.run(code, locals);
    }
  }
  return clones;
}

BoundingBox Cloner::bounding_box()
{
  return BoundingBox();  // TODO
}

std::string Cloner::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Cloner::clone() const
{
  return std::make_unique<Cloner>(*this);
}

Object::Flag Cloner::flags() const
{
  return Object::flags() | Flag::Convertable;
}

std::unique_ptr<Object> Cloner::convert()
{
  auto converted = std::make_unique<Empty>(scene());
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

}  // namespace omm
