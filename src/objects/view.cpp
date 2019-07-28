#include "objects/view.h"
#include "properties/vectorproperty.h"
#include <QObject>
#include "geometry/point.h"
#include "renderers/style.h"
#include "properties/triggerproperty.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "properties/boolproperty.h"

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
  add_property<BoolProperty>(OUTPUT_VIEW_PROPERTY_KEY, false)
      .set_label(QObject::tr("output").toStdString()).set_category(category);
}

View::View(const View &other) : Object(other)
{
  if (property(OUTPUT_VIEW_PROPERTY_KEY)->value<bool>()) {
    make_output_unique();
  }
}

std::string View::type() const { return TYPE; }
BoundingBox View::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
    const double l = -size.x/2.0;
    const double t = -size.y/2.0;
    const double r = size.x/2.0;
    const double b = size.y/2.0;
    const std::vector<Point> points { Point(Vec2f(l, t)), Point(Vec2f(l, b)),
                                      Point(Vec2f(r, b)), Point(Vec2f(r, t)) };
    const QPainterPath painter_path = Painter::path(points, is_closed());
    return BoundingBox((painter_path * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

View::Flag View::flags() const { return Flag::IsView; }
std::unique_ptr<Object> View::clone() const { return std::make_unique<View>(*this); }

void View::to_viewport()
{
  auto t = global_transformation(true).inverted().normalized();
  auto& viewport = Application::instance().main_window()->viewport();
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
  const auto sx = viewport.width() / size.x;
  const auto sy = viewport.height() / size.y;
  const auto s = std::abs(sx) < std::abs(sy) ? sx : sy;
  t = ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
  viewport.set_transformation(t);
}

void View::make_output_unique()
{
  const auto views = type_cast<View*>(scene()->object_tree.items());
  for (View* view : views) {
    auto& property = *view->property(OUTPUT_VIEW_PROPERTY_KEY);
    Property::NotificationBlocker blocker(property);
    property.set(view == this);
  }
}

void View::draw_handles(Painter &renderer) const
{
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
  renderer.set_style(ContourStyle(Colors::BLACK));
  renderer.painter->drawRect(QRectF(-size.x/2.0, -size.y/2.0, size.x, size.y));
}

void View::on_property_value_changed(Property& property, std::set<const void *> trace)
{
  if (&property == this->property(TO_VIEWPORT_PROPERTY_KEY)) { to_viewport(); }
  if (&property == this->property(FROM_VIEWPORT_PROPERTY_KEY)) { from_viewport(); }
  if (&property == this->property(OUTPUT_VIEW_PROPERTY_KEY)) { make_output_unique(); }
  Object::on_property_value_changed(property, trace);
}

void View::from_viewport()
{
  const auto& viewport = Application::instance().main_window()->viewport();
  const auto s = Vec2f(double(viewport.size().width()), double(viewport.size().height()));
  set_global_transformation(ObjectTransformation().translated(s/2.0).normalized(), false);
  property(SIZE_PROPERTY_KEY)->set(s);
}

}  // namespace omm
