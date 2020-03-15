#pragma once

#include <memory>
#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include <Qt>

namespace omm {

class Object;
class Scene;

class Tag
  : public PropertyOwner<Kind::Tag>
  , public AbstractFactory<QString, true, Tag, Object&>
{
public:
  explicit Tag(Object& owner);
  ~Tag();
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Tag", "Tag");
  std::unique_ptr<Tag> clone(Object& owner) const;
  std::unique_ptr<Tag> clone() const { return AbstractFactory::clone(); }
  Object* owner;
  virtual void evaluate() = 0;
  virtual void force_evaluate() { evaluate(); }
  Flag flags() const override;
};

void register_tags();
std::ostream& operator<<(std::ostream& ostream, const Tag& tag);

}  // namespace omm
