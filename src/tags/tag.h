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

  virtual bool run();

  static const std::string NAME_PROPERTY_KEY;
  std::string type() const override;
  std::string name() const override;

};

void register_tags();

}  // namespace omm
