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
void show_keybindings_dialog();
void previous_tool();

} // namespace omm::actions