#pragma once
#include "objects/path.h"

namespace omm
{

class Scene;
class Application;

}  // namespace omm

namespace omm::actions
{

void make_smooth(Application& app);
void make_linear(Application& app);
void remove_selected_points(Application& app);
void subdivide(Application& app);
void select_all(Application& app);
void deselect_all(Application& app);
void invert_selection(Application& app);
void remove_selection(Application& app);
void convert_objects(Application& app);
void remove_unused_styles(Application& app);

} // namespace omm::actions
