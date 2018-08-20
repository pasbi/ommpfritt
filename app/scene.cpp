#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include "object.h"
#include "external/json.hpp"

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : m_root(std::make_unique<RootObject>(*this))
{
  m_current = this;
}

Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
}

Scene::RootObject& Scene::root()
{
  return *m_root;
}

ObjectView Scene::root_view()
{
  return ObjectView(*m_root);
}

Scene* Scene::currentInstance()
{
  return m_current;
}

void Scene::reset()
{
  // ...
}

bool Scene::load(const nlohmann::json& data)
{
  return true;
  //m_root = ...
}

nlohmann::json Scene::save() const
{
  m_root->update_ids();
  return {
    { "root", m_root->to_json() }
  };
}