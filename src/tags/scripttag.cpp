#include "tags/scripttag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"

namespace py = pybind11;

namespace omm
{

ScriptTag::ScriptTag()
{
  add_property(RUN_PROPERTY_KEY, std::make_unique<BoolProperty>())
    .set_label("run").set_category("script"); // TODO actual value is ignored
  add_property(CODE_PROPERTY_KEY, std::make_unique<StringProperty>("", StringProperty::LineMode::MultiLine))
    .set_label("code").set_category("script");
}

ScriptTag::~ScriptTag()
{
}

std::string ScriptTag::type() const
{
  return TYPE;
}

QIcon ScriptTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_FileDialogListView);
}

bool ScriptTag::run() const
{
  const auto code = property(CODE_PROPERTY_KEY).value<std::string>();
  try {
    py::exec(code, py::globals());
    return true;
  } catch (const std::exception& e) {
    LOG(WARNING) << e.what();
    return false;
  }
}

void ScriptTag::on_property_value_changed(Property& property)
{
  if (&property == &this->property(RUN_PROPERTY_KEY)) {
    run();
  }
}

}  // namespace omm
