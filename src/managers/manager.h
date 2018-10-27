#pragma once

#include <unordered_set>
#include <memory>

namespace omm
{

class Scene;

/**
 * @brief Manager is the abstract base class for e.g., ObjectManager or PropertyManager.
 * A manager is usually closely tied to the ui, i.e., it creates a (dockable) window.
 */
class Manager // : public Gtk::Window // TODO dock window
{
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;

  template<typename ManagerT>
  static Manager& create_manager(Scene& scene)
  {
    std::unique_ptr<Manager> manager = std::make_unique<ManagerT>(scene);
    Manager& ref_manager = *manager;
    m_managers.insert(std::move(manager));
    return ref_manager;
  }

protected:
  explicit Manager(Scene& scene);

private:
  static std::unordered_set<std::unique_ptr<Manager>> m_managers;


  Scene& m_scene;

};

} // namespace omm

