#pragma once
#include "objects/path.h"

namespace omm
{

class Scene;

}  // namespace omm

namespace omm::actions
{


void make_smooth();
void make_linear();
void remove_selected_points();
void subdivide();
void evaluate();

} // namespace omm::actions