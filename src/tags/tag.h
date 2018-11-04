#pragma once

#include <memory>
#include "properties/hasproperties.h"
#include "external/json_fwd.hpp"
#include "objects/selectable.h"

namespace omm {

class Object;
class Scene;

class Tag : public HasProperties, public Selectable
{
public:
  explicit Tag(Object& owner);
  virtual ~Tag();
  Object& owner() const;

  virtual bool run();

  static const std::string NAME_PROPERTY_KEY;
  Scene& scene() const;

  nlohmann::json to_json() const;
  static std::unique_ptr<Tag> from_json(const nlohmann::json& json);

protected:
  DEFINE_CLASSNAME("tag")

private:
  Object& m_owner;
};

}  // namespace omm
