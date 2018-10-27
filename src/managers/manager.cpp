 #include "manager.h"

#include "managers/objectmanager.h"
#include "managers/propertymanager.h"

std::unordered_set<std::unique_ptr<omm::Manager>> omm::Manager::m_managers;

omm::Manager::Manager(Scene& scene)
  : m_scene(scene)
{
}
