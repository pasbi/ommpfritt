#pragma once

#include <QString>
#include <set>

namespace omm
{

class Scene;
class Application;
class Object;

}  // namespace omm

namespace omm::path_actions
{

std::set<QString> available_actions();
std::set<Object*> convert_objects(Application& app);
bool perform_action(Application& app, const QString& name);

}  // namespace omm::path_actions
