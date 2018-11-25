#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "serializers/jsonserializer.h"

namespace omm
{

const std::string Tag::NAME_PROPERTY_KEY = "name";

Tag::Tag()
{
  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<unnamed tag>") )
    .set_label(QObject::tr("Name").toStdString())
    .set_category(QObject::tr("tag").toStdString());
}

Tag::~Tag() { }

std::string Tag::name() const
{
  return property<std::string>(NAME_PROPERTY_KEY).value();
}

}  // namespace omm
