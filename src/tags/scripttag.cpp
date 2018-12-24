#include "tags/scripttag.h"
#include <QApplication>  // TODO only for icon testing
#include <QStyle>        // TODO only for icon testing
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"

namespace py = pybind11;

namespace omm
{

ScriptTag::ScriptTag(Object& owner)
  : Tag(owner)
{
  add_property<StringProperty>(CODE_PROPERTY_KEY, "", StringProperty::LineMode::MultiLine)
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

}  // namespace omm
