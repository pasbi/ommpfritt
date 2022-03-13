#pragma once

#include "common.h"
#include <set>

namespace omm
{

class Face;
class Scene;

class FaceSelection
{
public:
  FaceSelection(Scene& scene);
  [[nodiscard]] ::transparent_set<Face> faces() const;

private:
  Scene& m_scene;
};

}  // namespace omm
