#include "objects/empty.h"
#include "properties/optionsproperty.h"

namespace omm
{

Empty::Empty(Scene* scene) : Object(scene)
{
  const std::vector<std::string> options { "O1", "O2" };
  add_property<OptionsProperty>("hdhe")
    .set_options(options).set_label("hdhe").set_category("base");
}

void Empty::render(AbstractRenderer&, const Style&) { return; }
BoundingBox Empty::bounding_box() { return BoundingBox({ arma::vec2{0, 0} }); }
std::string Empty::type() const { return TYPE; }
std::unique_ptr<Object> Empty::clone() const { return std::make_unique<Empty>(*this); }

}  // namespace omm
