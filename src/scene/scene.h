#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include "python/objectview.h"
#include "external/json_fwd.hpp"

namespace omm
{

class Object;

class Scene
{

public:
  using RootObject = Object;
  Scene();
  ~Scene();

  RootObject& root();
  ObjectView root_view();

  static Scene* currentInstance();

  void reset();
  bool load(const nlohmann::json& data);
  nlohmann::json save() const;
 
private:
  std::unique_ptr<RootObject> m_root;
  static Scene* m_current;
};

}  // namespace omm

