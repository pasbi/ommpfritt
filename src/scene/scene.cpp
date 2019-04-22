#include "scene.h"
#include <random>
#include <cassert>
#include <QDebug>
#include <variant>
#include <QTimer>
#include <QMessageBox>
#include <fstream>

#include "objects/empty.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "tools/selecttool.h"
#include "logging.h"

namespace
{

template<typename StructureT, typename ItemsT>
void remove_items(omm::Scene& scene, StructureT& structure, const ItemsT& selection)
{
  using remove_command_type = omm::RemoveCommand<StructureT>;
  scene.submit<remove_command_type>(structure, selection);
}

constexpr auto ROOT_POINTER = "root";
constexpr auto STYLES_POINTER = "styles";

auto implicitely_selected_tags(const std::set<omm::AbstractPropertyOwner*>& selection)
{
  std::set<omm::AbstractPropertyOwner*> tags;
  for (auto* object : omm::kind_cast<omm::Object>(selection)) {
    for (auto* tag : object->tags.items()) {
      tags.insert(tag);
    }
  }
  return tags;
}


std::unique_ptr<omm::Style> make_default_style(omm::Scene* scene)
{
  auto default_style = std::make_unique<omm::Style>(scene);
  default_style->property(omm::Style::PEN_IS_ACTIVE_KEY).set(true);
  default_style->property(omm::Style::BRUSH_IS_ACTIVE_KEY).set(true);
  return default_style;
}

template<typename T> std::set<T*> filter_by_name(const std::set<T*>& set, const std::string& name)
{
  return ::filter_if(set, [name](const T* t) { return t->name() == name; });
}

}  // namespace

namespace omm
{

Scene::Scene(PythonEngine& python_engine)
  : object_tree(make_root(), this)
  , object_tree_adapter(*this, object_tree)
  , styles(this)
  , style_list_adapter(*this, styles)
  , python_engine(python_engine)
  , m_default_style(make_default_style(this))
  , tool_box(*this)
{
  using namespace std::string_literals;
  object_tree.root().property(Object::NAME_PROPERTY_KEY).set("_root_"s);
  for (auto kind : { Object::KIND, Tag::KIND, Style::KIND, Tool::KIND }) {
    m_item_selection[kind] = {};
  }
  tool_box.set_active_tool(SelectObjectsTool::TYPE);
}

std::unique_ptr<Object> Scene::make_root()
{
  class Root : public Empty
  {
  public:
    explicit Root(Scene* scene) : Empty(scene) {}
    void on_change(AbstractPropertyOwner* subject, int code, Property* property) override
    {
      Object::on_change(subject, code, property);
      if (code == Object::HIERARCHY_CHANGED) {
        scene()->invalidate();
      } else if (code == AbstractPropertyOwner::PROPERTY_CHANGED) {
        const auto* object = kind_cast<const Object*>(subject);
        assert(property != nullptr);
        assert(object != nullptr);
        if ( property == &object->property(Object::IS_VISIBLE_PROPERTY_KEY)
             || property == &object->property(Object::IS_ACTIVE_PROPERTY_KEY) )
        {
          scene()->invalidate();  // reset all the handles
        }
      }
    }
  };


  auto root = std::make_unique<Root>(this);
  root->register_observer(*this);
  return std::unique_ptr<Object>(root.release());
}

std::set<ReferenceProperty*>
Scene::find_reference_holders(const AbstractPropertyOwner& candidate) const
{
  std::set<ReferenceProperty*> reference_holders;
  for (const auto& property_owner : property_owners()) {
    const auto& property_map = property_owner->properties();
    for (const auto& key : property_map.keys()) {
      auto& property = *property_map.at(key);
      using value_type = ReferenceProperty::value_type;
      const auto variant = property.variant_value();
      if (const auto* value = std::get_if<value_type>(&variant); value != nullptr) {
        if (*value == &candidate) {
          reference_holders.insert(static_cast<ReferenceProperty*>(&property));
        }
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
  const auto notifier = std::mem_fn(&AbstractSimpleStructureObserver::structure_has_changed);
  Q_EMIT structure_changed();
  set_selection(::filter_if(m_selection, [this](auto* apo) {
    return contains(apo);
  }));
  tool_box.active_tool().on_scene_changed();
}

bool Scene::save_as(const std::string &filename)
{
  std::ofstream ofstream(filename);
  if (!ofstream) {
    LERROR <<  "Failed to open ofstream at '" << filename << "'.";
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

  LINFO << "Saved current scene to '" << filename << "'.";
  set_has_pending_changes(false);
  m_filename = filename;
  return true;
}

bool Scene::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (!ifstream) {
    LERROR << "Failed to open '" << filename << "'.";
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

    set_selection({});
    m_filename = filename;
    set_has_pending_changes(false);
    QTimer::singleShot(0, [ this, new_root=std::move(new_root),
                            styles=std::move(styles) ]() mutable
    {
      this->object_tree.replace_root(std::move(new_root));
      this->styles.set(std::move(styles));
    });

    return true;
  } catch (const AbstractDeserializer::DeserializeError& deserialize_error) {
    LERROR << "Failed to deserialize file at '" << filename << "'.";
    LINFO << deserialize_error.what();
  }
  return false;
}

void Scene::reset()
{
  set_has_pending_changes(false);
  set_selection({});
  QTimer::singleShot(0, [this]() {
    object_tree.replace_root(make_root());
    styles.set(std::vector<std::unique_ptr<Style>> {});
  });
  m_filename.clear();
}

std::string Scene::filename() const
{
  return m_filename;
}

void Scene::set_has_pending_changes(bool v)
{
  m_has_pending_changes = v;
}

void Scene::on_change(AbstractPropertyOwner *apo, int what, Property *property)
{
  Q_UNUSED(apo)
  Q_UNUSED(what)
  Q_UNUSED(property)
  Q_EMIT scene_changed();
}

bool Scene::has_pending_changes() const
{
  return m_has_pending_changes;
}

void Scene::submit(std::unique_ptr<Command> command)
{
  history.push(std::move(command));
  set_has_pending_changes(true);
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
  m_selection = selection;
  Q_EMIT selection_changed(m_selection);

  static const auto emit_selection_changed = [this](const auto& selection, const auto kind) {
    Q_EMIT selection_changed(selection, kind);

    switch (kind) {
    case AbstractPropertyOwner::Kind::Style:
      Q_EMIT style_selection_changed(kind_cast<Style>(selection));
      break;
    case AbstractPropertyOwner::Kind::Object:
      Q_EMIT object_selection_changed(kind_cast<Object>(selection));
      break;
    case AbstractPropertyOwner::Kind::Tag:
      Q_EMIT tag_selection_changed(kind_cast<Tag>(selection));
      break;
    case AbstractPropertyOwner::Kind::Tool:
      Q_EMIT tool_selection_changed(kind_cast<Tool>(selection));
      break;
    default:
      break;
    }
  };

  for (auto& kind : { AbstractPropertyOwner::Kind::Object, AbstractPropertyOwner::Kind::Style,
                      AbstractPropertyOwner::Kind::Tag, AbstractPropertyOwner::Kind::Tool })
  {
    if (selection.size() == 0) {
      m_item_selection.at(kind).clear();
      emit_selection_changed(m_selection, kind);
    } else {
      const auto item_selection = ::filter_if(selection, [kind](const auto* apo) {
        return apo->kind() == kind;
      });
      if (item_selection.empty()) {
        // selection is not empty but does not contain objects. Do not touch the object selection.
      } else {
        if (m_item_selection[kind] != item_selection) {
          m_item_selection[kind] = item_selection;
          emit_selection_changed(m_item_selection.at(kind), kind);
        }
      }
    }
  }

  tool_box.active_tool().on_selection_changed();
}

std::set<AbstractPropertyOwner*> Scene::selection() const { return m_selection; }

template<> std::set<Tag*> Scene::find_items<Tag>(const std::string& name) const
{
  return filter_by_name(tags(), name);
}

template<> std::set<Object*> Scene::find_items<Object>(const std::string& name) const
{
  return filter_by_name(object_tree.items(), name);
}

template<> std::set<Style*> Scene::find_items<Style>(const std::string& name) const
{
  return filter_by_name(styles.items(), name);
}

void Scene::evaluate_tags()
{
  for (Tag* tag : tags()) { tag->evaluate(); }
}

bool Scene::can_remove( QWidget* parent, std::set<AbstractPropertyOwner*> selection,
                                         std::set<Property*>& properties ) const
{
  selection = merge(selection, implicitely_selected_tags(selection));
  const auto reference_holder_map = find_reference_holders(selection);
  if (reference_holder_map.size() > 0) {
    const auto message = QObject::tr("There are %1 items being referenced by other items.\n"
                                     "Remove the refrenced items anyway?")
                                    .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning( parent, QObject::tr("Warning"), message,
                                                QMessageBox::YesToAll | QMessageBox::Cancel );
    switch (decision) {
    case QMessageBox::YesToAll:
    {
      const auto merge = [](std::set<Property*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      properties = std::accumulate( reference_holder_map.begin(), reference_holder_map.end(),
                                    std::set<Property*>(), merge );
      return true;
    }
    case QMessageBox::Cancel:
      return false;
    default:
      assert(false);
    }
  } else {
    return true;
  }
}

bool Scene::remove(QWidget* parent, const std::set<AbstractPropertyOwner*>& selection)
{
  std::set<Property *> properties;
  if (can_remove(parent, selection, properties)) {
    auto macro = history.start_macro(QObject::tr("Remove Selection"));
    if (properties.size() > 0) {
      using command_type = PropertiesCommand<ReferenceProperty>;
      submit<command_type>(properties, nullptr);
    }

    std::map<Object*, std::set<Tag*>> tag_map;
    for (Tag* tag : kind_cast<Tag>(selection)) {
      if (!::contains(selection, tag->owner)) { tag_map[tag->owner].insert(tag); }
    }
    for (auto [ owner, tags ] : tag_map) { ::remove_items(*this, owner->tags, tags); }
    ::remove_items(*this, styles, kind_cast<Style>(selection));
    ::remove_items(*this, object_tree, kind_cast<Object>(selection));

    return true;
  } else {
    return false;
  }
}

void Scene::update()
{
  object_tree.root().update_recursive();
}

bool Scene::contains(const AbstractPropertyOwner *apo) const
{
  switch (apo->kind()) {
  case AbstractPropertyOwner::Kind::Tag:
  {
    const auto tags = this->tags();
    return tags.end() != std::find(tags.begin(), tags.end(), static_cast<const Tag*>(apo));
  }
  case AbstractPropertyOwner::Kind::Object:
    return object_tree.contains(static_cast<const Object&>(*apo));
  case AbstractPropertyOwner::Kind::Style:
    return styles.contains(static_cast<const Style&>(*apo));
  default: return false;
  }
}

}  // namespace omm
