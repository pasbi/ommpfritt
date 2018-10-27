#pragma once

#include "scene/sceneobjectmodel.h"

#include "managers/manager.h"

namespace omm
{

class ObjectManager : public Manager
{
public:
  explicit ObjectManager(omm::Scene& scene);

private:
  // Gtk::TreeView m_tree_view;
  SceneObjectModel m_columns;
};

}  // namespace omm
