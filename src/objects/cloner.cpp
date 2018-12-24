#include "objects/cloner.h"

#include <QObject>

#include "properties/integerproperty.h"
#include "properties/stringproperty.h"
#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/pythonengine.h"

namespace omm
{

class Style;

Cloner::Cloner(Scene& scene) : Object(scene)
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

void Cloner::render(AbstractRenderer& renderer, const Style& style) const
{
  using namespace pybind11::literals;
  const auto n_children = this->n_children();
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();

  std::vector<std::unique_ptr<Object>> copies;
  copies.reserve(n_children);
  for (int i = 0; i < n_children; ++i) {
    copies.push_back(child(i % n_children).copy());
  }

  if (copies.size() > 0) {
    for (int i = 0; i < count; ++i) {
      auto& copy = *copies.at(i % copies.size());
      const auto locals = pybind11::dict( "id"_a=i,
                                          "count"_a=count,
                                          "copy"_a=ObjectWrapper::make(&copy),
                                          "scene"_a=SceneWrapper(&renderer.scene) );
      renderer.scene.python_engine.run(code, locals);
      renderer.push_transformation(copy.transformation());
      copy.render_recursive(renderer, style);
      renderer.pop_transformation();
    }
  }
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
