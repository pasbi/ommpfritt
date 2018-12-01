#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include <QDebug>

#include "objects/empty.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"
#include "properties/referenceproperty.h"

namespace
{

constexpr auto ROOT_POINTER = "root";

auto make_root()
{
  return std::make_unique<omm::Empty>();
}

template<typename T> bool is_selected(const T* t) { return t->is_selected(); }

template<typename SetA, typename SetB> SetA merge(SetA&& a, SetB&& b)
{
  a.insert(b.begin(), b.end());
  return a;
}

template<typename SetA, typename SetB, typename... Sets>
SetA merge(SetA&& a, SetB&& b, Sets&&... sets)
{
  return merge(merge(a, b), std::forward<Sets>(sets)...);
}


}  // namespace

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene() : m_root(make_root())
{
  m_root->property<StringProperty>(Object::NAME_PROPERTY_KEY).value() = "_root_";
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

  Observed<AbstractObjectTreeObserver>::for_each(
    [&parent, n] (auto* observer) { observer->beginInsertObject(parent, n); }
  );

  parent.adopt(std::move(object));

  Observed<AbstractObjectTreeObserver>::for_each(
    [] (auto* observer) { observer->endInsertObject(); }
  );

  invalidate_getter_cache();
  invalidate_selected_getter_cache();
}

Object& Scene::root() const
{
  return *m_root;
}

bool Scene::is_referenced(const AbstractPropertyOwner& candidate) const
{
  for (const auto property_owner : property_owners()) {
    const auto& properties = property_owner->properties();
    for (const auto& key : properties.keys()) {
      const auto* reference_property = properties.at(key)->cast<ReferenceProperty>();
      if (reference_property != nullptr && reference_property->value() == &candidate) {
        return true;
      }
    }
  }
  return false;
}

void Scene::selection_changed()
{
  const auto selected_objects = selection();

  Observed<AbstractSelectionObserver>::for_each(
    [selected_objects](auto* observer) { observer->set_selection(selected_objects); }
  );

  invalidate_selected_getter_cache();
}

void Scene::clear_selection()
{
  for (auto& o : root().all_descendants()) {
    o->set_selected(false);
    for (auto& t : o->tags()) {
      t->set_selected(false);
    }
  }
  selection_changed();
}

void Scene::move_object(MoveObjectTreeContext context)
{
  assert(context.is_valid());
  Object& old_parent = context.subject.get().parent();

  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginMoveObject(context); }
  );
  context.parent.get().adopt(old_parent.repudiate(context.subject), context.predecessor);
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endMoveObject(); }
  );

  invalidate_getter_cache();
  invalidate_selected_getter_cache();
}

void Scene::insert_object(OwningObjectTreeContext& context)
{
  assert(context.subject.owns());
  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginInsertObject(context); }
  );
  context.parent.get().adopt(context.subject.release(), context.predecessor);
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endInsertObject(); }
  );

  invalidate_getter_cache();
  invalidate_selected_getter_cache();
}

void Scene::remove_object(OwningObjectTreeContext& context)
{
  assert(!context.subject.owns());
  Observed<AbstractObjectTreeObserver>::for_each(
    [&context](auto* observer) { observer->beginRemoveObject(context.subject); }
  );
  context.subject.capture(context.parent.get().repudiate(context.subject));
  Observed<AbstractObjectTreeObserver>::for_each(
    [](auto* observer) { observer->endRemoveObject(); }
  );

  invalidate_getter_cache();
  invalidate_selected_getter_cache();
}

bool Scene::save_as(const std::string &filename)
{
  std::ofstream ofstream(filename);
  if (ofstream) {
    auto serializer = AbstractSerializer::make( "JSONSerializer",
                                                static_cast<std::ostream&>(ofstream) );
    root().serialize(*serializer, ROOT_POINTER);

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
    auto deserializer = AbstractDeserializer::make( "JSONDeserializer",
                                                    static_cast<std::istream&>(ifstream) );
    try {
      auto new_root = make_root();
      new_root->deserialize(*deserializer, ROOT_POINTER);
      replace_root(std::move(new_root));
    } catch (const AbstractDeserializer::DeserializeError& deserialize_error) {
      LOG(ERROR) << "Failed to deserialize file at '" << filename << "'.";
      LOG(INFO) << deserialize_error.what();
      return false;
    }

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
  replace_root(make_root());
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

StylePool& Scene::style_pool()
{
  return m_style_pool;
}

const StylePool& Scene::style_pool() const
{
  return m_style_pool;
}

void Scene::invalidate_getter_cache() const
{
  objects.invalidate();
  tags.invalidate();
  property_owners.invalidate();
}

void Scene::invalidate_selected_getter_cache() const
{
  selected_objects.invalidate();
  selected_tags.invalidate();
  selection.invalidate();
}

template<> std::set<Object*> Scene::TGetter<Object>::compute() const
{
  return m_self.root().all_descendants();

}

template<> std::set<Object*> Scene::SelectedTGetter<Object>::compute() const
{
  return ::filter_if(m_self.objects(), is_selected<Object>);
}

template<> std::set<Tag*> Scene::TGetter<Tag>::compute() const
{
  std::set<Tag*> tags;
  for (const auto object : m_self.objects()) {
    tags = merge(tags, object->tags());
  }
  return tags;
}

template<> std::set<Tag*> Scene::SelectedTGetter<Tag>::compute() const
{
  return ::filter_if(m_self.tags(), is_selected<Tag>);
}

template<> std::set<AbstractPropertyOwner*> Scene::TGetter<AbstractPropertyOwner>::compute() const
{
  // TODO add selected styles
  return merge( std::set<AbstractPropertyOwner*>(),
                m_self.selected_objects(), m_self.selected_tags());
}

template<>
std::set<AbstractPropertyOwner*> Scene::SelectedTGetter<AbstractPropertyOwner>::compute() const
{
  // TODO add styles
  return merge(std::set<AbstractPropertyOwner*>(), m_self.objects(), m_self.tags());
}

}  // namespace omm
