#include "tags/scripttag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "properties/triggerproperty.h"

#include "python/tagwrapper.h"
#include "python/scenewrapper.h"
#include "python/pythonengine.h"
#include "common.h"

constexpr auto default_script = R"(scale = this.owner().get("scale")[0]
scale = scale + 0.05
if scale > 2:
  scale = -scale
this.owner().set("scale", [scale, scale])
)";

namespace py = pybind11;

namespace omm
{

ScriptTag::ScriptTag(Object& owner)
  : Tag(owner)
{
  add_property<StringProperty>(CODE_PROPERTY_KEY, default_script)
    .set_mode(StringProperty::Mode::Code)
    .set_label("code").set_category("script");
  add_property<OptionsProperty>(UPDATE_MODE_PROPERTY_KEY)
    .set_options({ "per frame", "on request" })
    .set_label("update").set_category("script");
  add_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
    .set_label("evaluate").set_category("script");
}

std::string ScriptTag::type() const { return TYPE; }
AbstractPropertyOwner::Flag ScriptTag::flags() const { return Tag::flags() | Flag::HasScript; }
std::unique_ptr<Tag> ScriptTag::clone() const { return std::make_unique<ScriptTag>(*this); }

QIcon ScriptTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_FileDialogListView);
}

void ScriptTag::on_property_value_changed(Property& property)
{
  if (&property == &this->property(TRIGGER_UPDATE_PROPERTY_KEY)) { force_evaluate(); }
}

void ScriptTag::force_evaluate()
{
  Scene* scene = owner->scene();
  assert(scene != nullptr);
  using namespace py::literals;
  const auto code = property(ScriptTag::CODE_PROPERTY_KEY).value<std::string>();
  auto locals = py::dict("this"_a=TagWrapper::make(*this), "scene"_a=SceneWrapper(*scene));
  scene->python_engine.exec(code, locals, this);
}

void ScriptTag::evaluate()
{
  if (property(UPDATE_MODE_PROPERTY_KEY).value<std::size_t>() == 0) {
    force_evaluate();
  }
}

}  // namespace omm
