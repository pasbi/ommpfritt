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

Scene::Scene(Project& project)
  : m_root(std::make_unique<Object>(*this))
  , m_project(project)
{
  m_root->property<std::string>(Object::NAME_PROPERTY_KEY).value() = "_root_";
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

  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [&parent, n] (auto* observer) { observer->beginInsertObject(parent, n); }
  );

  parent.adopt(std::move(object));

  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [] (auto* observer) { observer->endInsertObject(); }
  );
}

Object& Scene::root() const
{
  return *m_root;
}

void Scene::selection_changed()
{
  const auto selected_objects = m_root->get_selected_children_and_tags();

  ObserverRegister<AbstractPropertyObserver>::for_each(
    [selected_objects](auto* observer) { observer->set_selection(selected_objects); }
  );
}

void Scene::move_object(MoveObjectTreeContext context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginMoveObject(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endMoveObject(); }
  );
}

void Scene::insert_object(CopyObjectTreeContext& context)
{
  assert(context.subject.owns());
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginInsertObject(context); }
  );
  context.parent.get().adopt(context.subject.release(), context.predecessor);
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endInsertObject(); }
  );
}

void Scene::remove_object(CopyObjectTreeContext& context)
{
  assert(!context.subject.owns());
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginRemoveObject(context.subject); }
  );
  assert(context.predecessor == context.subject.reference().predecessor());
  context.capture(context.parent.repudiate(context.subject));
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endRemoveObject(); }
  );
}

Project& Scene::project()
{
  return m_project;
}

}  // namespace omm
