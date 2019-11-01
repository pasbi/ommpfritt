#include "objects/imageobject.h"
#include "properties/stringproperty.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "mainwindow/viewport/viewport.h"
#include "properties/integerproperty.h"
#include "properties/optionsproperty.h"

namespace
{

bool is_paged_image(const QString& filename)
{
  return filename.endsWith(".pdf", Qt::CaseInsensitive);
}

}  // namespace

namespace omm
{

ImageObject::ImageObject(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("image");
  create_property<StringProperty>(FILEPATH_PROPERTY_KEY)
    .set_mode(StringProperty::Mode::FilePath)
    .set_label(QObject::tr("filename")).set_category(category);

  create_property<FloatProperty>(WIDTH_PROPERTY_KEY, 100)
    .set_range(0.0, FloatProperty::highest_possible_value)
    .set_label(QObject::tr("Width")).set_category(category);
  create_property<FloatProperty>(OPACITY_PROPERTY_KEY, 1.0)
    .set_range(0.0, 1.0).set_step(0.01)
    .set_label(QObject::tr("Opacity")).set_category(category);

  create_property<IntegerProperty>(PAGE_PROPERTY_KEY, 0)
    .set_range(0, IntegerProperty::highest_possible_value)
    .set_label(QObject::tr("Page")).set_category(category);

  create_property<OptionsProperty>(HANCHOR_PROPERTY_KEY, 1)
    .set_options({ QObject::tr("Left"),
                   QObject::tr("Center"),
                   QObject::tr("Right") })
    .set_label(QObject::tr("Horizontal")).set_category(category);

  create_property<OptionsProperty>(VANCHOR_PROPERTY_KEY, 1)
    .set_options({ QObject::tr("Top"),
                   QObject::tr("Center"),
                   QObject::tr("Bottom") })
    .set_label(QObject::tr("Vertical")).set_category(category);
  update();
}

void ImageObject::draw_object(Painter &renderer, const Style&, Painter::Options options) const
{
  Q_UNUSED(options)
  if (is_active()) {
    const auto path = property(FILEPATH_PROPERTY_KEY)->value<QString>();
    const auto opacity = property(OPACITY_PROPERTY_KEY)->value<double>();
    const auto width = property(WIDTH_PROPERTY_KEY)->value<double>();
    const auto page_num = is_paged_image(path) ? property(PAGE_PROPERTY_KEY)->value<int>() : 0;

    QPainter& painter = *renderer.painter;
    painter.save();
    painter.setOpacity(opacity);
    const auto key = std::pair(path, page_num);
    const QPicture& picture = renderer.image_cache.get(key);
    const auto s = width / picture.width();
    const auto aabb = picture.boundingRect();
    painter.scale(s, s);
    painter.translate(-aabb.topLeft());

    painter.drawPicture(pos(aabb.size()), picture);
    painter.restore();
  }
}

QString ImageObject::type() const { return TYPE; }
std::unique_ptr<Object> ImageObject::clone() const { return std::make_unique<ImageObject>(*this); }

QPointF ImageObject::pos(const QSizeF &size) const
{
  const auto dim = [](const double dim, const std::size_t option) {
    switch (option) {
    case 0: return 0.0;
    case 1: return -dim/2.0;
    case 2: return -dim;
    default: Q_UNREACHABLE();
    }
  };

  return QPointF( dim(size.width(), property(HANCHOR_PROPERTY_KEY)->value<std::size_t>()),
                  dim(size.height(), property(VANCHOR_PROPERTY_KEY)->value<std::size_t>()) );
}

BoundingBox ImageObject::bounding_box(const ObjectTransformation &transformation) const
{
  // implementing this is relly a problem.
  // The height of the image is not known at this point.
  return BoundingBox();
}

void ImageObject::on_property_value_changed(Property *property)
{
  if (   property == this->property(WIDTH_PROPERTY_KEY)
      || property == this->property(OPACITY_PROPERTY_KEY)
      || property == this->property(PAGE_PROPERTY_KEY)
      || property == this->property(HANCHOR_PROPERTY_KEY)
      || property == this->property(VANCHOR_PROPERTY_KEY))
  {
    update();
  } else if (property == this->property(FILEPATH_PROPERTY_KEY)) {
    this->property(PAGE_PROPERTY_KEY)->set_visible(is_paged_image(property->value<QString>()));
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
