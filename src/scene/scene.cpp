#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include "object.h"
#include "external/json.hpp"

omm::Scene* omm::Scene::m_current = nullptr;

omm::Scene::Scene()
  : m_root(std::make_unique<RootObject>(*this))
{
  m_current = this;
}

omm::Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
}

omm::Scene::RootObject& omm::Scene::root()
{
  return *m_root;
}

omm::ObjectView omm::Scene::root_view()
{
  return ObjectView(*m_root);
}

omm::Scene* omm::Scene::currentInstance()
{
  return m_current;
}

void omm::Scene::reset()
{
  // ...
}

bool omm::Scene::load(const nlohmann::json& data)
{
  return true;
  //m_root = ...
}

nlohmann::json omm::Scene::save() const
{
  m_root->update_ids();
  return {
    { "root", m_root->to_json() }
  };
}