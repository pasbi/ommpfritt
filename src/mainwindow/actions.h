#pragma once
#include "objects/path.h"

namespace omm
{

class Scene;

}  // namespace omm

namespace omm::actions
{


void make_smooth(Scene& scene);
void make_linear(Scene& scene);
void remove_selected_points(Scene& scene);
void subdivide(Scene& scene);
void evaluate(Scene& scene);

} // namespace omm::actions