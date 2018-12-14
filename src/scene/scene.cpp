#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <cassert>
#include <QDebug>

#include "objects/empty.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "tags/tag.h"

namespace
{

constexpr auto ROOT_POINTER = "root";
constexpr auto STYLES_POINTER = "styles";

auto make_root()
{
  return std::make_unique<omm::Empty>();
}

std::unique_ptr<omm::Style> make_default_style(omm::Scene* scene)
{
  auto default_style = std::make_unique<omm::Style>(scene);
  default_style->property<omm::BoolProperty>(omm::Style::PEN_IS_ACTIVE_KEY).set_value(true);
  default_style->property<omm::BoolProperty>(omm::Style::BRUSH_IS_ACTIVE_KEY).set_value(true);
  return default_style;
}

}  // namespace

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : object_tree(this, make_root())
  , object_tree_adapter(*this)
  , styles(this)
  , style_list_adapter(*this)
  , m_default_style(make_default_style(this))
{
  object_tree.root().property<StringProperty>(Object::NAME_PROPERTY_KEY).value() = "_root_";
  m_current = this;
}

Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
}

Scene* Scene::currentInstance()
{
  return m_current;
}

std::set<ReferenceProperty*>
Scene::find_reference_holders(const AbstractPropertyOwner& candidate) const
{
  std::set<ReferenceProperty*> reference_holders;
  for (const auto& property_owner : property_owners()) {
    const auto& property_map = property_owner->properties();
    for (const auto& key : property_map.keys()) {
      auto* reference_property = property_map.at(key)->cast<ReferenceProperty>();
      if (reference_property != nullptr && reference_property->value() == &candidate) {
        reference_holders.insert(reference_property);
      }
    }
  }
  return reference_holders;
}

std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>>
Scene::find_reference_holders(const std::set<AbstractPropertyOwner*>& candidates) const
{
  std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>> reference_holder_map;
  for (const auto* reference : candidates) {
    const auto reference_holders = find_reference_holders(*reference);
    if (reference_holders.size() > 0) {
      reference_holder_map.insert(std::make_pair(reference, reference_holders));
    }
  }
  return reference_holder_map;
}

void Scene::invalidate()
{
  m_tags_cache_is_dirty = true;
  Observed<AbstractSimpleStructureObserver>::for_each([](auto* observer){
    observer->structure_has_changed();
  });
}

bool Scene::save_as(const std::string &filename)
{
  std::ofstream ofstream(filename);
  if (!ofstream) {
    LOG(ERROR) << "Failed to open ofstream at '" << filename << "'";
    return false;
  }

  auto serializer = AbstractSerializer::make( "JSONSerializer",
                                              static_cast<std::ostream&>(ofstream) );
  object_tree.root().serialize(*serializer, ROOT_POINTER);

  serializer->start_array(styles.items().size(), Serializable::make_pointer(STYLES_POINTER));
  for (size_t i = 0; i < styles.items().size(); ++i) {
    styles.item(i).serialize(*serializer, Serializable::make_pointer(STYLES_POINTER, i));
  }
  serializer->end_array();

  LOG(INFO) << "Saved current scene to '" << filename << "'";
  set_has_pending_changes(false);
  m_filename = filename;
  return true;
}

bool Scene::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (!ifstream) {
    LOG(ERROR) << "Failed to open '" << filename << "'.";
    return false;
  }

  try
  {
    auto deserializer = AbstractDeserializer::make( "JSONDeserializer",
                                                    static_cast<std::istream&>(ifstream) );

    auto new_root = make_root();
    new_root->deserialize(*deserializer, ROOT_POINTER);

    const auto n_styles = deserializer->array_size(Serializable::make_pointer(STYLES_POINTER));
    std::vector<std::unique_ptr<Style>> styles;
    styles.reserve(n_styles);
    for (size_t i = 0; i < n_styles; ++i) {
      const auto style_pointer = Serializable::make_pointer(STYLES_POINTER, i);
      auto style = std::make_unique<Style>();
      style->deserialize(*deserializer, style_pointer);
      styles.push_back(std::move(style));
    }

    object_tree.replace_root(std::move(new_root));
    this->styles.set(std::move(styles));
    set_has_pending_changes(false);
    m_filename = filename;

    return true;
  } catch (const AbstractDeserializer::DeserializeError& deserialize_error) {
    LOG(ERROR) << "Failed to deserialize file at '" << filename << "'.";
    LOG(INFO) << deserialize_error.what();
  }
  return false;
}

void Scene::reset()
{
  set_has_pending_changes(false);
  object_tree.replace_root(make_root());
  styles.set(std::vector<std::unique_ptr<Style>> {});
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

std::set<Tag*> Scene::tags() const
{
  if (m_tags_cache_is_dirty) {
    m_tags_cache.clear();
    for (const auto& object : object_tree.items()) {
      m_tags_cache = merge(m_tags_cache, object->tags.items());
    }
  }
  return m_tags_cache;
}

std::set<AbstractPropertyOwner*> Scene::property_owners() const
{
  return merge(std::set<AbstractPropertyOwner*>(), object_tree.items(), tags(), styles.items());
}

Style& Scene::default_style() const
{
  return *m_default_style;
}

template<> typename SceneStructure<Object>::type& Scene::structure<Object>()
{
  return object_tree;
}

template<> const typename SceneStructure<Object>::type& Scene::structure<Object>() const
{
  return object_tree;
}

template<> typename SceneStructure<Style>::type& Scene::structure<Style>()
{
  return styles;
}

template<> const typename SceneStructure<Style>::type& Scene::structure<Style>() const
{
  return styles;
}

void Scene::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  const auto set_selection = [selection](auto* observer) { observer->set_selection(selection); };
  Observed<AbstractSelectionObserver>::for_each(set_selection);
}

}  // namespace omm
