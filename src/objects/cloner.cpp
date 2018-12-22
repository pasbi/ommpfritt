#include "objects/cloner.h"

#include <QObject>

#include "properties/integerproperty.h"
#include "properties/stringproperty.h"
#include "python/scenewrapper.h"
#include "python/objectwrapper.h"

namespace omm
{

class Style;

Cloner::Cloner()
{
  m_draw_children = false;
  add_property( COUNT_PROPERTY_KEY, std::make_unique<IntegerProperty>(3) )
    .set_label(QObject::tr("count").toStdString())
    .set_category(QObject::tr("Cloner").toStdString());
  add_property( CODE_PROPERTY_KEY,
                std::make_unique<StringProperty>("", StringProperty::LineMode::MultiLine) )
    .set_label(QObject::tr("code").toStdString())
    .set_category(QObject::tr("Cloner").toStdString());
}

void Cloner::render_instances(AbstractRenderer& renderer, const Style& style) const
{
  using namespace pybind11::literals;
  const auto n_children = this->n_children();
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();

  if (n_children > 0) {
    for (int i = 0; i < count; ++i) {
      Instance instance;
      instance.property(Instance::REFERENCE_PROPERTY_KEY).set(&child(i % n_children));
      const auto locals = pybind11::dict( "id"_a=i,
                                          "instance"_a=ObjectWrapper::make(&instance),
                                          "scene"_a=SceneWrapper(&renderer.scene)    );
      renderer.scene.python_engine.run(code, locals);
      instance.render_recursive(renderer, style);
    }
  }
}

void Cloner::render_copies(AbstractRenderer& renderer, const Style& style) const
{
  using namespace pybind11::literals;
  const auto n_children = this->n_children();
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();

  if (n_children > 0) {
    for (int i = 0; i < count; ++i) {
      auto copy = child(i % n_children).copy();
      const auto locals = pybind11::dict( "id"_a=i,
                                          "copy"_a=ObjectWrapper::make(copy.get()),
                                          "scene"_a=SceneWrapper(&renderer.scene) );
      renderer.scene.python_engine.run(code, locals);
      copy->render_recursive(renderer, style);
    }
  }
}

void Cloner::render(AbstractRenderer& renderer, const Style& style) const
{
  render_copies(renderer, style);
}

BoundingBox Cloner::bounding_box() const
{
  return BoundingBox();  // TODO
}

std::string Cloner::type() const
{
  return TYPE;
}

}  // namespace omm
