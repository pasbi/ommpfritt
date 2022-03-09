#include "scene.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <cassert>
#include <fstream>
#include <random>
#include <variant>

#include "removeif.h"
#include "commands/command.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "external/json.hpp"
#include "logging.h"
#include "mainwindow/exportoptions.h"
#include "objects/empty.h"
#include "objects/object.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "renderers/style.h"
#include "scene/sceneserializer.h"
#include "tags/nodestag.h"
#include "tags/tag.h"
#include "tools/selectobjectstool.h"
#include "tools/selecttool.h"

#include "animation/animator.h"
#include "color/namedcolors.h"
#include "keybindings/modeselector.h"
#include "main/application.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "scene/disjointpathpointsetforest.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/mailbox.h"
#include "scene/objecttree.h"
#include "scene/stylelist.h"
#include "scene/pointselection.h"
#include "tools/toolbox.h"

namespace
{

constexpr auto ROOT_POINTER = "root";
constexpr auto STYLES_POINTER = "styles";
constexpr auto ANIMATOR_POINTER = "animation";
constexpr auto NAMED_COLORS_POINTER = "colors";
constexpr auto EXPORT_OPTIONS_POINTER = "export_options";
constexpr auto JOINED_POINTS_POINTER =  "joined_points";

template<typename PropertyT, typename PropertyOwners>
std::set<PropertyT*>
find_properties(const PropertyOwners& property_owners,
                const std::function<bool(const typename PropertyT::value_type&)>& predicate)
{
  std::set<PropertyT*> properties;
  for (const auto& property_owner : property_owners) {
    const auto& property_map = property_owner->properties();
    for (const auto& key : property_map.keys()) {
      auto& property = *property_map.at(key);
      if (property.type() == PropertyT::TYPE()) {
        const auto variant = property.variant_value();
        const auto* value = std::get_if<typename PropertyT::value_type>(&variant);
        assert(value != nullptr);
        if (predicate(*value)) {
          properties.insert(static_cast<PropertyT*>(&property));
        }
      }
    }
  }
  return properties;
}

template<typename StructureT, typename ItemsT>
void remove_items(omm::Scene& scene, StructureT& structure, const ItemsT& selection)
{
  using remove_command_type = omm::RemoveCommand<StructureT>;
  scene.submit<remove_command_type>(structure, selection);
}

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
  return util::remove_if(set, [name](const T* t) { return t->name() != name; });
}

std::set<omm::AbstractPropertyOwner*> collect_apos_without_nodes(const omm::Scene& scene)
{
  auto apos = ::merge(std::set<omm::AbstractPropertyOwner*>(),
                      scene.object_tree().items(),
                      scene.styles().items(),
                      scene.tags());
  return apos;
}

}  // namespace

namespace omm
{
Scene::Scene()
    : point_selection(std::make_unique<PointSelection>(*this))
    , m_mail_box(new MailBox())
    , m_object_tree(new ObjectTree(make_root(), *this))
    , m_styles(new StyleList(*this))
    , m_history(new HistoryModel())
    , m_tool_box(new ToolBox(*this))
    , m_animator(new Animator(*this))
    , m_named_colors(new NamedColors())
    , m_export_options(new ExportOptions())
    , m_joined_points(new DisjointPathPointSetForest())
{
  object_tree().root().set_object_tree(object_tree());
  for (auto kind : {Object::KIND, Tag::KIND, Style::KIND, Tool::KIND, nodes::Node::KIND}) {
    m_item_selection[kind] = {};
  }
  connect(&history(), &HistoryModel::index_changed, &mail_box(), &MailBox::filename_changed);
  connect(&history(), &HistoryModel::index_changed, this, [this]() {
    const auto old_selection = selection();
    const auto new_selection = util::remove_if(old_selection, [this](const auto* apo) {
      return !contains(apo);
    });
    if (old_selection.size() > new_selection.size()) {
      set_selection(new_selection);
    }
  });
  connect(&mail_box(), &MailBox::selection_changed, this, &Scene::update_tool);
}

Scene::~Scene()
{
  history().disconnect();
  prepare_reset();
}

void Scene::polish()
{
  m_default_style = std::make_unique<Style>(this);

  connect(Application::instance().mode_selectors.at("scene_mode").get(),
          &ModeSelector::mode_changed,
          this,
          [this](int mode) { this->set_mode(static_cast<SceneMode>(mode)); });
}

void Scene::prepare_reset()
{
  Q_EMIT mail_box().about_to_reset();
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
  styles().set(std::deque<std::unique_ptr<Style>>{});
  tool_box().active_tool().reset();
}

std::unique_ptr<CycleGuard> Scene::make_cycle_guard(const Object* guarded)
{
  return std::make_unique<CycleGuard>(m_cycle_guarded_objects, guarded);
}

std::set<ReferenceProperty*>
Scene::find_reference_holders(const AbstractPropertyOwner& candidate) const
{
  return find_properties<ReferenceProperty>(property_owners(), [&candidate](auto&& reference) {
    return reference == &candidate;
  });
}

std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>>
Scene::find_reference_holders(const std::set<AbstractPropertyOwner*>& candidates) const
{
  std::map<const AbstractPropertyOwner*, std::set<ReferenceProperty*>> reference_holder_map;
  for (const auto* reference : candidates) {
    const auto reference_holders = find_reference_holders(*reference);
    if (!reference_holders.empty()) {
      reference_holder_map.insert(std::make_pair(reference, reference_holders));
    }
  }
  return reference_holder_map;
}

std::set<ColorProperty*> Scene::find_named_color_holders(const QString& name) const
{
  return find_properties<ColorProperty>(property_owners(), [name](auto&& color) {
    return color.model() == Color::Model::Named && color.name() == name;
  });
}

const ExportOptions& Scene::export_options() const
{
  return *m_export_options;
}

void Scene::set_export_options(const ExportOptions& export_options)
{
  if (*m_export_options != export_options) {
    *m_export_options = export_options;
    m_has_pending_changes = true;
    Q_EMIT mail_box().filename_changed();
  }
}

DisjointPathPointSetForest& Scene::joined_points() const
{
  return *m_joined_points;
}

bool Scene::save_as(const QString& filename)
{
  return SceneSerialization{*this}.save(filename);
}

bool Scene::load_from(const QString& filename)
{
  return SceneSerialization{*this}.load(filename);
}

void Scene::serialize(serialization::SerializerWorker& serializer)
{
  object_tree().root().serialize(*serializer.sub(ROOT_POINTER));

  serializer.sub(STYLES_POINTER)->set_value(styles().ordered_items(), [](const auto* style, auto& worker_i) {
    style->serialize(worker_i);
  });

  m_animator->serialize(*serializer.sub(ANIMATOR_POINTER));
  m_named_colors->serialize(*serializer.sub(NAMED_COLORS_POINTER));
  export_options().serialize(*serializer.sub(EXPORT_OPTIONS_POINTER));
  m_joined_points->serialize(*serializer.sub(JOINED_POINTS_POINTER));
}

void Scene::deserialize(serialization::DeserializerWorker& deserializer)
{
  auto new_root = make_root();
  new_root->deserialize(*deserializer.sub(ROOT_POINTER));

  std::deque<std::unique_ptr<Style>> styles;
  deserializer.sub(STYLES_POINTER)->get_items([this, &styles](auto& worker_i) {
    auto style = std::make_unique<Style>(this);
    style->deserialize(worker_i);
    styles.push_back(std::move(style));
  });

  history().set_saved_index();

  object_tree().replace_root(std::move(new_root));
  this->styles().set(std::move(styles));
  animator().invalidate();

  object_tree().root().update_recursive();

  animator().deserialize(*deserializer.sub(ANIMATOR_POINTER));
  named_colors().deserialize(*deserializer.sub(NAMED_COLORS_POINTER));
  m_export_options->deserialize(*deserializer.sub(EXPORT_OPTIONS_POINTER));
  m_joined_points->deserialize(*deserializer.sub(JOINED_POINTS_POINTER));
}

void Scene::reset()
{
  set_selection({});
  prepare_reset();
  history().reset();
  history().set_saved_index();
  object_tree().replace_root(make_root());
  styles().set({});
  m_filename.clear();
  animator().invalidate();
  m_named_colors->clear();
  Q_EMIT mail_box().filename_changed();
}

QString Scene::filename() const
{
  return m_filename;
}

void Scene::submit(std::unique_ptr<Command> command) const
{
  if (command != nullptr) {
    history().push(std::move(command));
    Q_EMIT mail_box().filename_changed();
  }
}

bool Scene::has_pending_changes() const
{
  return m_has_pending_changes || history().has_pending_changes();
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
  auto apos = collect_apos_without_nodes(*this);
  apos = ::merge(apos, collect_nodes(apos));
  return apos;
}

Style& Scene::default_style() const
{
  return *m_default_style;
}

void Scene::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  // If you'd ever chose to emit signals only if the selection actually changes,
  // be aware that members like Scene::emit_selection_changed_signal rely on the current
  // behaviour and adapt them.

  m_selection = selection;

  const auto emit_selection_changed = [this](const auto& selection, const auto kind) {
    auto& mail_box = this->mail_box();
    Q_EMIT mail_box.kind_selection_changed(selection, kind);

    switch (kind) {
    case Kind::Style:
      Q_EMIT mail_box.style_selection_changed(kind_cast<Style>(selection));
      break;
    case Kind::Object:
      Q_EMIT mail_box.object_selection_changed(kind_cast<Object>(selection));
      break;
    case Kind::Tag:
      Q_EMIT mail_box.tag_selection_changed(kind_cast<Tag>(selection));
      break;
    case Kind::Tool:
      Q_EMIT mail_box.tool_selection_changed(kind_cast<Tool>(selection));
      break;
    case Kind::Node:
      Q_EMIT mail_box.node_selection_changed(kind_cast<nodes::Node>(selection));
      break;
    default:
      break;
    }
  };

  for (const auto& kind : {Kind::Object, Kind::Style, Kind::Tag, Kind::Tool}) {
    if (selection.empty()) {
      m_item_selection.at(kind).clear();
      emit_selection_changed(m_selection, kind);
    } else {
      const auto item_selection = util::remove_if(selection, [kind](const auto* apo) {
        return apo->kind != kind;
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

  Q_EMIT mail_box().selection_changed(m_selection);
}

std::set<AbstractPropertyOwner*> Scene::selection() const
{
  return m_selection;
}

void Scene::emit_selection_changed_signal()
{
  set_selection(m_selection);
}

std::unique_ptr<Object> Scene::make_root()
{
  auto root = std::make_unique<Empty>(this);
  root->property(Object::NAME_PROPERTY_KEY)->set(QString("_root_"));
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

void Scene::evaluate_tags() const
{
  for (Tag* tag : tags()) {
    tag->evaluate();
  }
}

std::set<nodes::Node*> Scene::collect_nodes(const std::set<AbstractPropertyOwner*>& owners)
{
  std::set<nodes::Node*> nodes;
  for (auto&& apo : owners) {
    if (!!(apo->flags() & Flag::HasNodes)) {
      const auto& nodes_owner = dynamic_cast<const nodes::NodesOwner&>(*apo);
      if (const auto& node_model = nodes_owner.node_model(); node_model.is_enabled()) {
        nodes = ::merge(nodes, node_model.nodes());
      }
    }
  }
  return nodes;
}

std::set<nodes::Node*> Scene::collect_nodes() const
{
  return collect_nodes(collect_apos_without_nodes(*this));
}

bool Scene::can_remove(QWidget* parent,
                       std::set<AbstractPropertyOwner*> selection,
                       std::set<Property*>& properties) const
{
  selection = merge(selection, implicitely_selected_tags(selection));
  const auto reference_holder_map = find_reference_holders(selection);
  if (!reference_holder_map.empty()) {
    const auto message = QObject::tr("There are %1 items being referenced by other items.\n"
                                     "Remove the referenced items anyway?")
                             .arg(reference_holder_map.size());
    const auto decision = QMessageBox::warning(parent,
                                               QObject::tr("Warning"),
                                               message,
                                               QMessageBox::YesToAll | QMessageBox::Cancel);
    switch (decision) {
    case QMessageBox::YesToAll: {
      const auto merge = [](std::set<Property*> accu, const auto& v) {
        accu.insert(v.second.begin(), v.second.end());
        return accu;
      };
      properties = std::accumulate(reference_holder_map.begin(),
                                   reference_holder_map.end(),
                                   std::set<Property*>(),
                                   merge);
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
  std::set<Property*> properties;
  if (can_remove(parent, selection, properties)) {
    [[maybe_unused]] auto macro = history().start_macro(QObject::tr("Remove Selection"));
    if (!properties.empty()) {
      using command_type = PropertiesCommand<ReferenceProperty>;
      submit<command_type>(properties, nullptr);
    }

    std::map<Object*, std::set<Tag*>> tag_map;
    for (Tag* tag : kind_cast<Tag>(selection)) {
      if (!selection.contains(tag->owner)) {
        tag_map[tag->owner].insert(tag);
      }
    }
    for (auto&& [owner, tags] : tag_map) {
      ::remove_items(*this, owner->tags, tags);
    }
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
  m_tool_box->active_tool().reset();
}

SceneMode Scene::current_mode() const
{
  return m_mode;
}

void Scene::set_mode(SceneMode mode)
{
  if (m_mode != mode) {
    m_mode = mode;
    m_tool_box->set_scene_mode(mode);
    Application::instance().mode_selectors.at("scene_mode")->set_mode(static_cast<int>(mode));
  }
}

bool Scene::contains(const AbstractPropertyOwner* apo) const
{
  switch (apo->kind) {
  case Kind::Tag: {
    const auto tags = this->tags();
    // the std::set::find does not allow keys of type `const Tag*`.
    // NOLINTNEXTLINE(performance-inefficient-algorithm)
    return tags.end() != std::find(tags.begin(), tags.end(), dynamic_cast<const Tag*>(apo));
  }
  case Kind::Node: {
    const auto nodes = this->collect_nodes();
    // the std::set::find does not allow keys of type `const Tag*`.
    // NOLINTNEXTLINE(performance-inefficient-algorithm)
    return nodes.end() != std::find(nodes.begin(), nodes.end(), dynamic_cast<const nodes::Node*>(apo));
  }
  case Kind::Object:
    return object_tree().contains(dynamic_cast<const Object&>(*apo));
  case Kind::Style:
    return styles().contains(dynamic_cast<const Style&>(*apo));
  case Kind::Tool:
    return m_tool_box->tools().contains(apo);
  default:
    return false;
  }
}

}  // namespace omm
