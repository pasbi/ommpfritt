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
  , public CopyCreatable<Tag, Object&>
{
public:
  explicit Tag(Object& owner);
  virtual ~Tag() = default;
  virtual QIcon icon() const = 0;
  static constexpr auto TYPE = "Tag";
  std::unique_ptr<Tag> copy() const override;
  Object& owner;

};

void register_tags();

}  // namespace omm
