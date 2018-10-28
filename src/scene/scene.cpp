#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include <QDebug>

#include "object.h"
#include "external/json.hpp"

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : m_root(std::make_unique<Object>(*this))
{
  m_current = this;
}

Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
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

void Scene::insert_object(std::unique_ptr<Object> object, Object& parent)
{
  size_t n = parent.children().size();

  for (auto&& adapter : m_adapters) {
    adapter->beginInsertObjects(parent, n, n);
  }

  parent.adopt(std::move(object));

  for (auto&& adapter : m_adapters) {
    adapter->endInsertObjects();
  }

}

Object& Scene::root() const
{
  return *m_root;
}

void Scene::register_object_tree_adapter(AbstractObjectTreeAdapter& adapter)
{
  assert(m_adapters.count(&adapter) == 0);
  m_adapters.insert(&adapter);
}

void Scene::unregister_object_tree_adapter(AbstractObjectTreeAdapter& adapter)
{
  assert(m_adapters.count(&adapter) == 1);
  m_adapters.erase(m_adapters.find(&adapter));
}


}  // namespace omm
