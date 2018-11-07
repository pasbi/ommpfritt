#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include <QDebug>

#include "object.h"
#include "external/json.hpp"
#include "properties/typedproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : m_root(std::make_unique<Object>())
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

void Scene::insert_object(OwningObjectTreeContext& context)
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

void Scene::remove_object(OwningObjectTreeContext& context)
{
  assert(!context.subject.owns());
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginRemoveObject(context.subject); }
  );
  assert(context.predecessor == context.subject.reference().predecessor());
  context.subject.capture(context.parent.get().repudiate(context.subject));
  ObserverRegister<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endRemoveObject(); }
  );
}

bool Scene::save_as(const std::string &filename)
{
  std::ofstream ofstream(filename);
  if (ofstream) {
    auto serializer = Serializing::AbstractSerializer::make("JSONSerializer", *this);
    serializer->serialize(ofstream);

    LOG(INFO) << "Saved current scene to '" << filename << "'";
    set_has_pending_changes(false);
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open ofstream at '" << filename << "'";
    return false;
  }
}

bool Scene::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (ifstream) {
    auto deserializer = Serializing::AbstractDeserializer::make("JSONDeserializer", *this);
    deserializer->deserialize(ifstream);

    // TODO load aux_scene. If no error occured, swap with m_scene.

    set_has_pending_changes(false);
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open '" << filename << "'.";
    return false;
  }
}

void Scene::reset()
{
  set_has_pending_changes(false);
  //
}

std::string Scene::filename() const
{
  return m_filename;
}

void Scene::set_has_pending_changes(bool v)
{
  m_has_pending_changes = v;
}

bool Scene::has_pending_changes() const
{
  return m_has_pending_changes;
}

void Scene::submit(std::unique_ptr<Command> command)
{
  m_undo_stack.push(command.release());
  set_has_pending_changes(true);
}

QUndoStack& Scene::undo_stack()
{
  return m_undo_stack;
}

std::unique_ptr<Object> Scene::replace_root(std::unique_ptr<Object> new_root)
{
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  return old_root;
}


}  // namespace omm
