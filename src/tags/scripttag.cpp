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

}  // namespace omm
