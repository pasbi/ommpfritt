#include "tags/scripttag.h"
#include "external/pybind11/embed.h"

#include "common.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle>  // TODO only for icon testing

constexpr auto default_script = R"(scale = this.owner().get("scale")[0]
scale = scale + 0.05
if scale > 2:
  scale = -scale
this.owner().set("scale", [scale, scale])
)";

namespace py = pybind11;

namespace omm
{
ScriptTag::ScriptTag(Object& owner) : Tag(owner)
{
  create_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
      .set_mode(StringProperty::Mode::Code)
      .set_label(QObject::tr("code"))
      .set_category(QObject::tr("script"));
  create_property<OptionProperty>(UPDATE_MODE_PROPERTY_KEY, 0)
      .set_options({QObject::tr("on request"), QObject::tr("per frame")})
      .set_label(QObject::tr("update"))
      .set_category(QObject::tr("script"));
  create_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
      .set_label(QObject::tr("evaluate"))
      .set_category(QObject::tr("script"));
}

QString ScriptTag::type() const
{
  return TYPE;
}
Flag ScriptTag::flags() const
{
  return Tag::flags() | Flag::HasScript;
}

void ScriptTag::on_property_value_changed(Property* property)
{
  if (property == this->property(TRIGGER_UPDATE_PROPERTY_KEY)) {
    force_evaluate();
  }
}

void ScriptTag::force_evaluate()
{
  Scene* scene = owner->scene();
  assert(scene != nullptr);
  using namespace py::literals;
  const auto code = property(ScriptTag::CODE_PROPERTY_KEY)->value<QString>();
  auto locals = py::dict("this"_a = TagWrapper::make(*this), "scene"_a = SceneWrapper(*scene));
  PythonEngine::instance().exec(code, locals, this);
  owner->update();
}

void ScriptTag::evaluate()
{
  if (property(UPDATE_MODE_PROPERTY_KEY)->value<std::size_t>() == 1) {
    force_evaluate();
  }
}

}  // namespace omm
