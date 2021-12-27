#pragma once

#include <QString>
#include <set>

namespace omm
{
class Scene;
class Application;

}  // namespace omm

namespace omm::path_actions
{

std::set<QString> available_actions();
bool perform_action(Application& app, const QString& name);

}  // namespace omm::path_actions
