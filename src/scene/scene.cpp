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

  AdapterRegister<AbstractObjectTreeAdapter>::for_each(
    [&parent, n] (auto* adapter) { adapter->beginInsertObjects(parent, n, n); }
  );

  parent.adopt(std::move(object));

  AdapterRegister<AbstractObjectTreeAdapter>::for_each(
    [] (auto* adapter) { adapter->endInsertObjects(); }
  );
}

Object& Scene::root() const
{
  return *m_root;
}

void Scene::selection_changed()
{
  const auto selected_objects = m_root->get_selected_children_and_tags();

  AdapterRegister<AbstractPropertyAdapter>::for_each(
    [selected_objects](auto* adapter) { adapter->set_selection(selected_objects); }
  );
}


}  // namespace omm
