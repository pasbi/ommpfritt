#include "tags/tag.h"

#include <memory>
#include <QObject>

#include "objects/object.h"
#include "external/json.hpp"

namespace omm
{

const std::string Tag::NAME_PROPERTY_KEY = "name";

Tag::Tag(Object& owner)
  : m_owner(owner)
{
  add_property<StringProperty>( NAME_PROPERTY_KEY,
                                QObject::tr("Name").toStdString(),
                                QObject::tr("Tag").toStdString(),
                                "<Unnamed Tag>" );
}

Tag::~Tag()
{

}

Object& Tag::owner() const
{
  return m_owner;
}

Scene& Tag::scene() const
{
  return m_owner.scene();
}

bool Tag::run()
{
  return false;
}

nlohmann::json Tag::to_json() const
{
  auto json = HasProperties::to_json();
  return json;
}

std::unique_ptr<Tag> from_json(const nlohmann::json& json, Object& owner)
{
  return std::make_unique<Tag>(owner);
}

}  // namespace omm
