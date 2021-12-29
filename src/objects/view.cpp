#include "objects/view.h"
#include "geometry/point.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "preferences/uicolors.h"
#include "properties/boolproperty.h"
#include "properties/floatvectorproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/painter.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include <QObject>

namespace omm
{
View::View(Scene* scene) : Object(scene)
{
  static constexpr double DEFAULT_SIZE = 100.0;
  static const auto category = QObject::tr("view");
  create_property<FloatVectorProperty>(SIZE_PROPERTY_KEY, Vec2f(DEFAULT_SIZE, DEFAULT_SIZE))
      .set_label(QObject::tr("size"))
      .set_category(category);
  create_property<TriggerProperty>(TO_VIEWPORT_PROPERTY_KEY)
      .set_label(QObject::tr("to viewport"))
      .set_category(category);
  create_property<TriggerProperty>(FROM_VIEWPORT_PROPERTY_KEY)
      .set_label(QObject::tr("from viewport"))
      .set_category(category);
  create_property<BoolProperty>(OUTPUT_VIEW_PROPERTY_KEY, false)
      .set_label(QObject::tr("output"))
      .set_category(category);
  update();
}

View::View(const View& other) : Object(other)
{
  if (property(OUTPUT_VIEW_PROPERTY_KEY)->value<bool>()) {
    make_output_unique();
  }
}

QString View::type() const
{
  return TYPE;
}

void View::draw_object(Painter&, const Style&, const PainterOptions&) const
{
  // do nothing
}

BoundingBox View::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active()) {
    const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
    const double l = -size.x / 2.0;
    const double t = -size.y / 2.0;
    const double r = size.x / 2.0;
    const double b = size.y / 2.0;
    QPainterPath path;
    path.addRect(QRectF{QPointF{l, t}, QPointF{r, b}});
    return BoundingBox{(path * transformation.to_qtransform()).boundingRect()};
  } else {
    return BoundingBox{};
  }
}

Flag View::flags() const
{
  return Flag::IsView;
}

void View::to_viewport()
{
  auto t = global_transformation(Space::Scene).inverted().normalized();
  auto& viewport = Application::instance().main_window()->viewport();
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
  const auto sx = viewport.width() / size.x;
  const auto sy = viewport.height() / size.y;
  const auto s = std::abs(sx) < std::abs(sy) ? sx : sy;
  t = ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
  viewport.set_transformation(t);
  viewport.update();
}

void View::make_output_unique()
{
  const auto views = type_casts<View*>(scene()->object_tree().items());
  for (View* view : views) {
    Property* property = view->property(OUTPUT_VIEW_PROPERTY_KEY);
    const bool new_value = property->value<bool>() && view == this;
    if (new_value != property->value<bool>()) {
      property->set(new_value);
    }
  }
}

void View::draw_handles(Painter& renderer) const
{
  const auto size = property(SIZE_PROPERTY_KEY)->value<Vec2f>();
  const auto status = is_active() ? QPalette::Active : QPalette::Inactive;
  QPen pen;
  pen.setColor(ui_color(status, "Handle", "camera-outline"));
  pen.setCosmetic(true);
  renderer.painter->save();
  renderer.painter->setPen(pen);
  renderer.painter->setBrush(ui_color(status, "Handle", "camera-fill"));
  renderer.painter->drawRect(QRectF(-size.x / 2.0, -size.y / 2.0, size.x, size.y));
  renderer.painter->restore();
}

void View::on_property_value_changed(Property* property)
{
  if (property == this->property(TO_VIEWPORT_PROPERTY_KEY)) {
    to_viewport();
  }
  if (property == this->property(FROM_VIEWPORT_PROPERTY_KEY)) {
    from_viewport();
  }
  if (property == this->property(OUTPUT_VIEW_PROPERTY_KEY)) {
    make_output_unique();
  }
  Object::on_property_value_changed(property);
}

void View::from_viewport()
{
  const auto& viewport = Application::instance().main_window()->viewport();
  const auto s = Vec2f(double(viewport.size().width()), double(viewport.size().height()));
  set_global_transformation(ObjectTransformation().translated(s / 2.0).normalized(),
                            Space::Viewport);
  property(SIZE_PROPERTY_KEY)->set(s);
}

}  // namespace omm
