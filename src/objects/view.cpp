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
  static const auto category = QObject::tr("view").toStdString();
  add_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, Vec2f(100.0, 100.0) )
    .set_label(QObject::tr("size").toStdString()).set_category(category);
  add_property<TriggerProperty>(TO_VIEWPORT_PROPERTY_KEY)
    .set_label(QObject::tr("to viewport").toStdString()).set_category(category);
  add_property<TriggerProperty>(FROM_VIEWPORT_PROPERTY_KEY)
    .set_label(QObject::tr("from viewport").toStdString()).set_category(category);
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
  const auto size = property(SIZE_PROPERTY_KEY).value<Vec2f>();
  const auto w = size.x/2;
  const auto h = size.y/2;
  const auto points = std::vector { Point(Vec2f(-w, -h)), Point(Vec2f( w, -h)),
                                    Point(Vec2f( w,  h)), Point(Vec2f(-w,  h)) };

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
  const auto s = Vec2f(double(viewport.size().width()), double(viewport.size().height()));
  set_global_transformation(ObjectTransformation().translated(s/2.0).normalized(), false);
  property(SIZE_PROPERTY_KEY).set(s);
}

ObjectTransformation View::transformation() const
{
  auto t = global_transformation(true).inverted().normalized();
  auto& viewport = Application::instance().main_window()->viewport();
  const auto size = property(SIZE_PROPERTY_KEY).value<Vec2f>();
  const auto sx = viewport.width() / size.x;
  const auto sy = viewport.height() / size.y;
  const auto s = std::abs(sx) < std::abs(sy) ? sx : sy;
  t = ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
  return t;
}

}  // namespace omm
