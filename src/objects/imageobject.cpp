#include "objects/imageobject.h"
#include "properties/stringproperty.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "mainwindow/viewport/viewport.h"

namespace omm
{

ImageObject::ImageObject(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("image").toStdString();
  add_property<StringProperty>(FILEPATH_PROPERTY_KEY)
    .set_mode(StringProperty::Mode::FilePath)
    .set_label(QObject::tr("filename").toStdString()).set_category(category);

  add_property<FloatProperty>(WIDTH_PROPERTY_KEY, 100)
    .set_range(0.0, FloatPropertyLimits::upper)
    .set_label(QObject::tr("Width").toStdString()).set_category(category);
  add_property<FloatProperty>(OPACITY_PROPERTY_KEY, 1.0)
    .set_range(0.0, 1.0).set_step(0.01)
    .set_label(QObject::tr("Opacity").toStdString()).set_category(category);
}

void ImageObject::draw_object(AbstractRenderer& renderer, const Style&)
{
  if (is_active()) {
    const auto path = property(FILEPATH_PROPERTY_KEY).value<std::string>();
    const auto width = property(WIDTH_PROPERTY_KEY).value<double>();
    const auto opacity = property(OPACITY_PROPERTY_KEY).value<double>();
    renderer.draw_image(path, Vec2f::o(), width, opacity);
  }
}

std::string ImageObject::type() const { return TYPE; }
std::unique_ptr<Object> ImageObject::clone() const { return std::make_unique<ImageObject>(*this); }
BoundingBox ImageObject::bounding_box()
{
  // implementing this is relly a problem.
  // The height of the image is not known at this point.
  return BoundingBox();
}

}  // namespace omm
