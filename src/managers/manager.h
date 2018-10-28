#pragma once

#include <unordered_set>
#include <memory>
#include <QDockWidget>

namespace omm
{

class Scene;

/**
 * @brief Manager is the abstract base class for e.g., ObjectManager or PropertyManager.
 * A manager is usually closely tied to the ui, i.e., it creates a (dockable) window.
 */
class Manager : public QDockWidget
{
  Q_OBJECT
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;

protected:
  explicit Manager(Scene& scene);

  Scene& m_scene;
};

} // namespace omm

