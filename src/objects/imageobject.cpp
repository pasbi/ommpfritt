#include "objects/imageobject.h"
#include "properties/stringproperty.h"

namespace omm
{

ImageObject::ImageObject(Scene* scene) : Object(scene)
{
  add_property<StringProperty>(FILEPATH_PROPERTY_KEY)
    .set_mode(StringProperty::Mode::FilePath)
    .set_label("filename").set_category("image");
}

void ImageObject::render(AbstractRenderer& renderer, const Style& style)
{
  const auto path = property(FILEPATH_PROPERTY_KEY).value<std::string>();
  renderer.draw_image(path, arma::vec2{0.0, 0.0}, arma::vec2{1.0, 1.0});
}

std::string ImageObject::type() const { return TYPE; }
std::unique_ptr<Object> ImageObject::clone() const { return std::make_unique<ImageObject>(*this); }
BoundingBox ImageObject::bounding_box() { return BoundingBox(); }

}  // namespace omm
