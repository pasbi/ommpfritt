#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"

const std::string omm::Tag::NAME_PROPERTY_KEY = "name";

omm::Tag::Tag(omm::Object& owner)
  : m_owner(owner)
{
  add_property<StringProperty>( NAME_PROPERTY_KEY,
                                QObject::tr("Name").toStdString(),
                                QObject::tr("Tag").toStdString(),
                                "<Unnamed Tag>" );
}

omm::Tag::~Tag()
{

}

omm::Object& omm::Tag::owner() const
{
  return m_owner;
}

omm::Scene& omm::Tag::scene() const
{
  return m_owner.scene();
}

bool omm::Tag::run()
{
  return false;
}

nlohmann::json omm::Tag::to_json() const
{
  auto json = HasProperties::to_json();
  return json;
}