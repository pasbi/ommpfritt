#pragma once

#include <memory>
#include "properties/hasproperties.h"
#include "external/json_fwd.hpp"
#include "objects/selectable.h"
#include "abstractfactory.h"

namespace omm {

class Object;
class Scene;

class Tag
  : public HasProperties
  , public Selectable
  , public AbstractFactory<std::string, Tag>
{
public:
  explicit Tag();
  virtual ~Tag();

  virtual bool run() { return true; }

  static const std::string NAME_PROPERTY_KEY;
  std::string name() const override;
  std::unique_ptr<Tag> copy() const;

};

void register_tags();

}  // namespace omm
