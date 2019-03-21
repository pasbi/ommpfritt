#include "objects/view.h"
#include "properties/vectorproperty.h"
#include <QObject>
#include "geometry/point.h"
#include "renderers/style.h"
#include "properties/triggerproperty.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"

namespace omm
{

View::View(Scene* scene) : Object(scene)
{
  add_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, arma::vec2 { 100.0, 100.0 } )
    .set_label(QObject::tr("size").toStdString())
    .set_category(QObject::tr("view").toStdString());
  add_property<TriggerProperty>(TO_VIEWPORT_PROPERTY_KEY)
    .set_label(QObject::tr("to viewport").toStdString())
    .set_category(QObject::tr("view").toStdString());
  add_property<TriggerProperty>(FROM_VIEWPORT_PROPERTY_KEY)
    .set_label(QObject::tr("from viewport").toStdString())
    .set_category(QObject::tr("view").toStdString());
}

std::string View::type() const { return TYPE; }
BoundingBox View::bounding_box() { return BoundingBox(); }
View::Flag View::flags() const { return Flag::IsView; }
std::unique_ptr<Object> View::clone() const { return std::make_unique<View>(*this); }

void View::to_viewport()
{
  auto& viewport = Application::instance().main_window()->viewport();
  viewport.set_transformation(transformation());
}

void View::render(AbstractRenderer& renderer, const Style&)
{
  renderer.set_category(AbstractRenderer::Category::Handles);
  const auto size = property(SIZE_PROPERTY_KEY).value<VectorPropertyValueType<arma::vec2>>();
  const auto w = size(0)/2;
  const auto h = size(1)/2;
  const auto points = std::vector { Point(arma::vec2{-w, -h}), Point(arma::vec2{ w, -h}),
                                    Point(arma::vec2{ w,  h}), Point(arma::vec2{-w,  h}) };

  const auto style = ContourStyle(Color::BLACK);
  renderer.draw_spline(points, style, true);
}

void View::on_property_value_changed(Property& property)
{
  if (&property == &this->property(TO_VIEWPORT_PROPERTY_KEY)) { to_viewport(); }
  if (&property == &this->property(FROM_VIEWPORT_PROPERTY_KEY)) { from_viewport(); }
}

void View::from_viewport()
{
  const auto& viewport = Application::instance().main_window()->viewport();
  auto t = viewport.viewport_transformation().inverted();
  const auto s = arma::vec2{ double(viewport.size().width()), double(viewport.size().height()) };
  set_global_transformation(ObjectTransformation().translated(s/2.0).normalized(), false);
  property(SIZE_PROPERTY_KEY).set(s);
}

ObjectTransformation View::transformation() const
{
  auto t = global_transformation(true).inverted().normalized();
  auto& viewport = Application::instance().main_window()->viewport();
  const auto size = property(SIZE_PROPERTY_KEY).value<VectorPropertyValueType<arma::vec2>>();
  const auto sx = viewport.width() / size(0);
  const auto sy = viewport.height() / size(1);
  const auto s = std::abs(sx) < std::abs(sy) ? sx : sy;
  t = ObjectTransformation().scaled(arma::vec2{ s, s }).apply(t);
  return t;
}

}  // namespace omm
