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
  void set_selected(const Face& face, bool is_selected);
  void select(const Face& face);
  void deselect(const Face& face);
  [[nodiscard]] bool is_selected(const Face& face);
  void clear();

private:
  ::transparent_set<Face> m_selection;
};

}  // namespace omm
