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
void select_all();
void deselect_all();
void invert_selection();
void delete_objects();
void convert_objects();

} // namespace omm::actions
