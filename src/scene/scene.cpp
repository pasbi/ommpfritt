#include "scene.h"
#include <random>
#include <cassert>
#include <QDebug>
#include <variant>
#include <QTimer>
#include <QMessageBox>
#include <fstream>
#include <QApplication>

#include "objects/empty.h"
#include "external/json.hpp"
#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "serializers/abstractserializer.h"
#include "commands/command.h"
#include "properties/referenceproperty.h"
#include "properties/colorproperty.h"
#include "renderers/style.h"
#include "tags/tag.h"
#include "objects/object.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "tools/selecttool.h"
#include "tools/selectobjectstool.h"
#include "logging.h"

#include "scene/history/historymodel.h"
#include "tools/toolbox.h"
#include "scene/stylelist.h"
#include "scene/objecttree.h"
#include "scene/messagebox.h"
#include "animation/animator.h"
#include "color/namedcolors.h"

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
constexpr auto ANIMATOR_POINTER = "animation";
constexpr auto NAMED_COLORS_POINTER = "colors";

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

template<typename T> std::set<T*> filter_by_name(const std::set<T*>& set, const QString& name)
{
  return ::filter_if(set, [name](const T* t) { return t->name() == name; });
}

}  // namespace

namespace omm
{

Scene::Scene(PythonEngine& python_engine)
  : python_engine(python_engine)
  , point_selection(*this)
  , m_message_box(new MessageBox())
  , m_object_tree(new ObjectTree(make_root(), *this))
  , m_default_style(new Style(this))
  , m_styles(new StyleList(*this))
  , m_history(new HistoryModel())
  , m_tool_box(new ToolBox(*this))
  , m_animator(new Animator(*this))
  , m_named_colors(new NamedColors())
{
  object_tree().root().set_object_tree(object_tree());
  for (auto kind : { Object::KIND, Tag::KIND, Style::KIND, Tool::KIND }) {
    m_item_selection[kind] = {};
  }
  connect(&history(), SIGNAL(index_changed()), &message_box(), SIGNAL(filename_changed()));
  connect(&message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(update_tool()));
}

Scene::~Scene()
{
  history().disconnect();
  prepare_reset();
}

void Scene::prepare_reset()
{
  set_selection({});

  // make sure that there are no references (via ReferenceProperties) across objects.
  // the references might be destructed after the referenced objects have been deleted.
  // that leads to fucked-up states, undefined behavior, etc.
  for (auto* o : object_tree().items()) {
    for (auto* p : o->properties().values()) {
      if (auto* ref_prop = type_cast<ReferenceProperty*>(p)) {
        ref_prop->set(nullptr);
      }
    }
  }

  auto root = make_root();
  object_tree().replace_root(std::move(root));
  styles().set(std::vector<std::unique_ptr<Style>> {});
  tool_box().active_tool().reset();
}

std::unique_ptr<CycleGuard> Scene::make_cycle_guard(const Object *guarded)
{
  return std::make_unique<CycleGuard>(m_cycle_guarded_objects, guarded);
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

std::set<ColorProperty*> Scene::find_named_color_holders(const QString& name) const
{
  std::set<ColorProperty*> properties;
  for (const auto& property_owner : property_owners()) {
    const auto& property_map = property_owner->properties();
    for (const auto& key : property_map.keys()) {
      auto& property = *property_map.at(key);
      using value_type = ColorProperty::value_type;
      const auto variant = property.variant_value();
      if (const auto* value = std::get_if<value_type>(&variant); value != nullptr) {
        if (value->model() == Color::Model::Named && value->name() == name) {
          properties.insert(static_cast<ColorProperty*>(&property));
        }
      }
    }
  }
  return properties;
}

bool Scene::save_as(const QString &filename)
{
  std::ofstream ofstream(filename.toStdString());
  if (!ofstream) {
    LERROR <<  "Failed to open ofstream at '" << filename << "'.";
    return false;
  }

  auto serializer = AbstractSerializer::make( "JSONSerializer",
                                              static_cast<std::ostream&>(ofstream) );
  object_tree().root().serialize(*serializer, ROOT_POINTER);

  serializer->start_array(styles().items().size(), Serializable::make_pointer(STYLES_POINTER));
  for (size_t i = 0; i < styles().items().size(); ++i) {
    styles().item(i).serialize(*serializer, Serializable::make_pointer(STYLES_POINTER, i));
  }
  serializer->end_array();

  animator().serialize(*serializer, ANIMATOR_POINTER);
  named_colors().serialize(*serializer, NAMED_COLORS_POINTER);

  LINFO << "Saved current scene to '" << filename << "'.";
  history().set_saved_index();
  m_filename = filename;
  Q_EMIT message_box().filename_changed();
  return true;
}

bool Scene::load_from(const QString &filename)
{
  assert(selection().size() == 0);
  prepare_reset();
  history().reset();
  std::ifstream ifstream(filename.toStdString());
  if (!ifstream) {
    LERROR << "Failed to open '" << filename << "'.";
    return false;
  }

  auto error_handler = [this, filename](const QString& msg) {
    LERROR << "Failed to deserialize file at '" << filename << "'.";
    LINFO << msg;
    animator().invalidate();
    reset();
  };

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
      auto style = std::make_unique<Style>(this);
      style->deserialize(*deserializer, style_pointer);
      styles.push_back(std::move(style));
    }

    set_selection({});
    m_filename = filename;
    history().set_saved_index();
    Q_EMIT message_box().filename_changed();

    this->object_tree().replace_root(std::move(new_root));
    this->styles().set(std::move(styles));
    animator().invalidate();

    object_tree().root().update_recursive();

    animator().deserialize(*deserializer, ANIMATOR_POINTER);

    named_colors().deserialize(*deserializer, NAMED_COLORS_POINTER);
    return true;
  } catch (const AbstractDeserializer::DeserializeError& deserialize_error) {
    error_handler(deserialize_error.what());
  } catch (const nlohmann::json::exception& exception) {
    error_handler(exception.what());
  }

  return false;
}

void Scene::reset()
{
  assert(selection().size() == 0);
  prepare_reset();
  history().reset();
  history().set_saved_index();
  object_tree().replace_root(make_root());
  styles().set(std::vector<std::unique_ptr<Style>> {});
  m_filename.clear();
  animator().invalidate();
  Q_EMIT message_box().filename_changed();
}

QString Scene::filename() const
{
  return m_filename;
}

void Scene::submit(std::unique_ptr<Command> command)
{
  if (command != nullptr) {
    history().push(std::move(command));
    Q_EMIT message_box().filename_changed();
  }
}

std::set<Tag*> Scene::tags() const
{
  std::set<Tag*> tags;
  for (const auto& object : object_tree().items()) {
    tags = merge(tags, object->tags.items());
  }
  return tags;
}

std::set<AbstractPropertyOwner*> Scene::property_owners() const
{
  return merge(std::set<AbstractPropertyOwner*>(), object_tree().items(), tags(), styles().items());
}

Style& Scene::default_style() const
{
  return *m_default_style;
}

void Scene::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  m_selection = selection;

  static const auto emit_selection_changed = [this](const auto& selection, const auto kind) {
    Q_EMIT message_box().selection_changed(selection, kind);

    switch (kind) {
    case AbstractPropertyOwner::Kind::Style:
      Q_EMIT message_box().selection_changed(kind_cast<Style>(selection));
      break;
    case AbstractPropertyOwner::Kind::Object:
      Q_EMIT message_box().selection_changed(kind_cast<Object>(selection));
      break;
    case AbstractPropertyOwner::Kind::Tag:
      Q_EMIT message_box().selection_changed(kind_cast<Tag>(selection));
      break;
    case AbstractPropertyOwner::Kind::Tool:
      Q_EMIT message_box().selection_changed(kind_cast<Tool>(selection));
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

  Q_EMIT message_box().selection_changed(m_selection);
}

std::set<AbstractPropertyOwner*> Scene::selection() const { return m_selection; }

std::unique_ptr<Object> Scene::make_root()
{
  auto root = std::make_unique<Empty>(this);
  root->property(Object::NAME_PROPERTY_KEY)->set(QStringLiteral("_root_"));
  return root;
}

template<> std::set<Tag*> Scene::find_items<Tag>(const QString& name) const
{
  return filter_by_name(tags(), name);
}

template<> std::set<Object*> Scene::find_items<Object>(const QString& name) const
{
  return filter_by_name(object_tree().items(), name);
}

template<> std::set<Style*> Scene::find_items<Style>(const QString& name) const
{
  return filter_by_name(styles().items(), name);
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
                                     "Remove the referenced items anyway?")
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
      Q_UNREACHABLE();
      return true;
    }
  } else {
    return true;
  }
}

bool Scene::remove(QWidget* parent, const std::set<AbstractPropertyOwner*>& selection)
{
  std::set<Property *> properties;
  if (can_remove(parent, selection, properties)) {
    auto macro = history().start_macro(QObject::tr("Remove Selection"));
    if (properties.size() > 0) {
      using command_type = PropertiesCommand<ReferenceProperty>;
      submit<command_type>(properties, nullptr);
    }

    std::map<Object*, std::set<Tag*>> tag_map;
    for (Tag* tag : kind_cast<Tag>(selection)) {
      if (!::contains(selection, tag->owner)) { tag_map[tag->owner].insert(tag); }
    }
    for (auto [ owner, tags ] : tag_map) { ::remove_items(*this, owner->tags, tags); }
    ::remove_items(*this, styles(), kind_cast<Style>(selection));
    ::remove_items(*this, object_tree(), kind_cast<Object>(selection));
    set_selection({});
    return true;
  } else {
    return false;
  }
}

void Scene::update_tool()
{
  tool_box().active_tool().reset();
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
    return object_tree().contains(static_cast<const Object&>(*apo));
  case AbstractPropertyOwner::Kind::Style:
    return styles().contains(static_cast<const Style&>(*apo));
  default: return false;
  }
}

}  // namespace omm
