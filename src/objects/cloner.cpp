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
  using namespace pybind11::literals;
  const auto n_children = this->n_children();
  const auto count = property(COUNT_PROPERTY_KEY).value<int>();
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();

  // TODO ProceduralPath works differently.
  // the cloner script is evaluated once for each clone individually, the procedural path script
  // is evaluated once for all points.
  // both approaches have pros and cons. The cloner-approach is simpler, it does not require a loop
  // in the python code. The path-approach gives more control, i.e. an expensive pre-computation
  // only needs to be performed once for all points.
  // However, implementing the procedural path approach for cloner introduces another drawback:
  // `count`-clones would be required to be kept in memory at once. If `count` is very large, that
  // might be a problem. With the current approach, only `n_children` objects must be in memory at
  // once.
  // I'd prefer to have the same approach in both procedural path and cloner, however, I like the
  // cloner approach too much to drop it.
  // Short term solution: have two different approaches
  // Long term solution: implement procedural path approach here next to current approach,
  //  introduce a OptionProperty s.t. user can chose. Consider to implement (optional)
  //  cloner-approach in procedural path, too.

  std::vector<std::unique_ptr<Object>> copies;
  copies.reserve(n_children);
  for (int i = 0; i < n_children; ++i) {
    copies.push_back(child(i % n_children).clone());
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

std::unique_ptr<Object> Cloner::clone() const
{
  return std::make_unique<Cloner>(*this);
}

}  // namespace omm
