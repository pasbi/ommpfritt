#include "tag.h"
#include "object.h"
#include "external/json.hpp"
#include <memory>

const std::string omm::Tag::NAME_PROPERTY_KEY = "name";

omm::Tag::Tag(omm::Object& owner)
  : m_owner(owner)
{
  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<Unnamed Tag>") );
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
  return {
    { "properties", property_map().to_json() }
  };
}