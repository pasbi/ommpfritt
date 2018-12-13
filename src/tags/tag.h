#pragma once

#include <memory>
#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "aspects/copycreatable.h"

namespace omm {

class Object;
class Scene;

class Tag
  : public PropertyOwner<AbstractPropertyOwner::Kind::Tag>
  , public CopyCreatable<Tag>
{
public:
  virtual ~Tag() = default;

  virtual bool run() { return true; }
  virtual QIcon icon() const = 0;

  void set_owner(Object* owner);
  Object* owner() const;

private:
  Object* m_owner;

};

void register_tags();

}  // namespace omm
