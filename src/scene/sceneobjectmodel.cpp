#include "sceneobjectmodel.h"
#include <glog/logging.h>

omm::SceneObjectModel::SceneObjectModel(Scene& scene)
  : m_scene(scene)
{
  LOG(INFO) << "CREATE som";
}

omm::SceneObjectModel::~SceneObjectModel()
{
  LOG(INFO) << "DELETE som";
}