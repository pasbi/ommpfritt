#include "tag.h"
#include "object.h"
#include <memory>

const std::string Tag::NAME_PROPERTY_KEY = "name";

Tag::Tag(Object& owner)
  : m_owner(owner)
{
  add_property( NAME_PROPERTY_KEY,
                std::make_unique<StringProperty>("<Unnamed Tag>") );
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