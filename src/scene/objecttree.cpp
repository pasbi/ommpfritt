#include "scene/objecttree.h"
#include "commands/addcommand.h"
#include "commands/movetagscommand.h"
#include "commands/propertycommand.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include "properties/referenceproperty.h"
#include "properties/stringproperty.h"
#include "propertyownermimedata.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tags/styletag.h"

namespace
{
using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;

template<typename T> T* last(const std::deque<T*>& ts)
{
  return ts.empty() ? nullptr : ts.back();
}

template<typename T>
void drop_tags(omm::Scene& scene,
               omm::Object& object,
               omm::Tag* predecessor,
               const std::vector<omm::Tag*>& tags,
               Qt::DropAction action,
               T&& make_context)
{
  if (!tags.empty()) {
    switch (action) {
    case Qt::CopyAction: {
      auto macro = scene.history().start_macro(QObject::tr("copy tags", "ObjectTreeAdapter"));
      for (auto* tag : tags) {
        scene.submit<AddTagCommand>(object.tags, make_context(tag));
      }
      break;
    }
    case Qt::MoveAction:
      scene.submit<omm::MoveTagsCommand>(tags, object, predecessor);
      break;
    default:
      break;
    }
  }
}

void drop_tags_onto_object(omm::Scene& scene,
                           omm::Object& object,
                           const std::vector<omm::Tag*>& tags,
                           Qt::DropAction action)
{
  const auto make_context = [&object](const omm::Tag* tag) { return tag->clone(object); };
  drop_tags(scene, object, last(object.tags.ordered_items()), tags, action, make_context);
}

void drop_tags_behind(omm::Scene& scene,
                      omm::Object& object,
                      omm::Tag* current_tag_predecessor,
                      const std::vector<omm::Tag*>& tags,
                      Qt::DropAction action)
{
  if (current_tag_predecessor != nullptr && current_tag_predecessor->owner != &object) {
    current_tag_predecessor = last(object.tags.ordered_items());
  }
  const auto make_context = [&object, current_tag_predecessor](const omm::Tag* tag) {
    return omm::ListOwningContext<omm::Tag>(tag->clone(object), current_tag_predecessor);
  };
  drop_tags(scene, object, current_tag_predecessor, tags, action, make_context);
}

void drop_style_onto_object(omm::Scene& scene,
                            omm::Object& object,
                            const std::vector<omm::Style*>& styles)
{
  if (!styles.empty()) {
    const QString label = QObject::tr("set styles tags", "ObjectTreeAdapter");
    [[maybe_unused]] auto macro = scene.history().start_macro(label);
    for (auto* style : styles) {
      auto style_tag = std::make_unique<omm::StyleTag>(object);
      style_tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY)->set(style);
      scene.submit<AddTagCommand>(object.tags, std::move(style_tag));
    }
  }
}

}  // namespace

namespace omm
{
ObjectTree::ObjectTree(std::unique_ptr<Object> root, Scene& scene)
    : ItemModelAdapter<ObjectTree, Object, QAbstractItemModel>(scene, *this),
      m_root(std::move(root)), m_scene(scene)
{
}

Object& ObjectTree::root() const
{
  return *m_root;
}

bool ObjectTree::contains(const Object& t) const
{
  const auto& root = static_cast<const TreeElement<Object>&>(*m_root);
  return root.is_ancestor_of(t);
}

void ObjectTree::move(ObjectTreeMoveContext& context)
{
  assert(context.is_valid());

  Object& old_parent = context.subject.get().tree_parent();
  Object& new_parent = context.parent.get();
  const auto old_pos = m_scene.object_tree().position(context.subject);
  const auto new_pos = m_scene.object_tree().insert_position(context.predecessor);
  const QModelIndex old_parent_index = m_scene.object_tree().index_of(old_parent);
  const QModelIndex new_parent_index = m_scene.object_tree().index_of(new_parent);

  beginMoveRows(old_parent_index, old_pos, old_pos, new_parent_index, new_pos);
  auto item = old_parent.repudiate(context.subject);
  const auto pos = this->insert_position(context.predecessor);
  context.parent.get().adopt(std::move(item), pos);
  m_item_cache_is_dirty = true;
  endMoveRows();
  Q_EMIT m_scene.mail_box().object_moved(old_parent, new_parent, context.get_subject());
  Q_EMIT expand_item(new_parent_index);
}

void ObjectTree::insert(ObjectTreeOwningContext& context)
{
  assert(context.subject.owns());

  const auto row = this->insert_position(context.predecessor);
  const QModelIndex parent_index = m_scene.object_tree().index_of(context.parent);
  beginInsertRows(parent_index, row, row);
  context.parent.get().adopt(context.subject.release(), row);
  m_item_cache_is_dirty = true;
  endInsertRows();
  Q_EMIT m_scene.mail_box().object_inserted(context.parent.get(), context.get_subject());
}

void ObjectTree::remove(ObjectTreeOwningContext& context)
{
  assert(!context.subject.owns());
  const Object& subject = context.subject;
  const int row = m_scene.object_tree().position(subject);
  const QModelIndex parent_index = m_scene.object_tree().index_of(subject.tree_parent());
  beginRemoveRows(parent_index, row, row);
  context.subject.capture(context.parent.get().repudiate(context.subject));
  m_item_cache_is_dirty = true;
  endRemoveRows();
  Q_EMIT m_scene.mail_box().object_removed(context.parent.get(), context.get_subject());
}

std::unique_ptr<Object> ObjectTree::remove(Object& t)
{
  const int row = m_scene.object_tree().position(t);
  const QModelIndex parent_index = m_scene.object_tree().index_of(t.tree_parent());
  beginRemoveRows(parent_index, row, row);
  assert(!t.is_root());
  Object& parent = t.tree_parent();
  auto item = parent.repudiate(t);
  m_item_cache_is_dirty = true;
  endRemoveRows();
  Q_EMIT m_scene.mail_box().object_removed(parent, t);
  return item;
}

std::unique_ptr<Object> ObjectTree::replace_root(std::unique_ptr<Object> new_root)
{
  beginResetModel();
  auto old_root = std::move(m_root);
  m_root = std::move(new_root);
  m_item_cache_is_dirty = true;
  endResetModel();
  Q_EMIT m_scene.mail_box().scene_reseted();
  return old_root;
}

std::set<Object*> ObjectTree::items() const
{
  if (m_item_cache_is_dirty) {
    m_item_cache_is_dirty = false;
    m_item_cache = root().all_descendants();
  }
  return m_item_cache;
}

std::size_t ObjectTree::position(const Object& item) const
{
  return item.position();
}

const Object* ObjectTree::predecessor(const Object& sibling) const
{
  assert(!sibling.is_root());
  const auto pos = position(sibling);
  if (pos == 0) {
    return nullptr;
  } else {
    return &sibling.tree_parent().tree_child(pos - 1);
  }
}

QModelIndex ObjectTree::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  return createIndex(row, column, &item_at(parent).tree_child(row));
}

QModelIndex ObjectTree::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  const Object& parent_item = item_at(index);
  if (parent_item.is_root()) {
    return QModelIndex();
  } else {
    return index_of(parent_item.tree_parent());
  }
}

int ObjectTree::rowCount(const QModelIndex& parent) const
{
  return item_at(parent).n_children();
}

int ObjectTree::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 3;
}

QVariant ObjectTree::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  switch (index.column()) {
  case 0:
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return item_at(index).name();
    case Qt::DecorationRole:
      return QIcon(IconProvider::pixmap(item_at(index).type()));
    }
  }
  return QVariant();
}

bool ObjectTree::setData(const QModelIndex& index, const QVariant& value, int role)
{
  Q_UNUSED(role)
  switch (index.column()) {
  case 0: {
    Property* property = item_at(index).property(Object::NAME_PROPERTY_KEY);
    const auto svalue = value.toString();
    if (property->value<QString>() != svalue) {
      scene.submit<PropertiesCommand<StringProperty>>(std::set{property}, svalue);
      return true;
    } else {
      return false;
    }
  }
  }

  return false;
}

Object& ObjectTree::item_at(const QModelIndex& index) const
{
  if (index.isValid()) {
    assert(index.internalPointer() != nullptr);
    return *static_cast<Object*>(index.internalPointer());
  } else {
    return structure.root();
  }
}

QModelIndex ObjectTree::index_of(Object& object) const
{
  if (object.is_root()) {
    return QModelIndex();
  } else {
    return createIndex(scene.object_tree().position(object), 0, &object);
  }
}

Qt::ItemFlags ObjectTree::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return Qt::ItemIsDropEnabled;
  }

  switch (index.column()) {
  case 0:
    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled
           | Qt::ItemIsDropEnabled;
  case 2:
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  default:
    return Qt::ItemIsEnabled;
  }
}

QVariant ObjectTree::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0:
      return QObject::tr("object", "ObjectTreeAdapter");
    case 1:
      return QObject::tr("is visible", "ObjectTreeAdapter");
    case 2:
      return QObject::tr("tags", "ObjectTreeAdapter");
    }
  }

  return QVariant();
}

bool ObjectTree::dropMimeData(const QMimeData* data,
                              Qt::DropAction action,
                              int row,
                              int column,
                              const QModelIndex& parent)
{
  if (ItemModelAdapter::dropMimeData(data, action, row, column, parent)) {
    return true;
  } else {
    const auto* pdata = qobject_cast<const PropertyOwnerMimeData*>(data);
    if (pdata != nullptr) {
      if (const auto tags = pdata->tags(); !tags.empty()) {
        if (parent.column() == OBJECT_COLUMN) {
          assert(parent.isValid());  // otherwise, column was < 0.
          drop_tags_onto_object(scene, item_at(parent), tags, action);
          return true;
        } else if (parent.column() == TAGS_COLUMN) {
          assert(tags.front() != current_tag_predecessor || action != Qt::MoveAction);
          drop_tags_behind(scene, item_at(parent), current_tag_predecessor, tags, action);
          return true;
        }
      }
      if (const auto styles = pdata->styles(); !styles.empty()) {
        if (parent.column() == OBJECT_COLUMN && column < 0) {
          assert(parent.isValid());  // otherwise, column was < 0
          drop_style_onto_object(scene, item_at(parent), styles);
        } else if (parent.column() == TAGS_COLUMN && column < 0) {
          assert(current_tag->type() == StyleTag::TYPE);
          assert(styles.size() == 1);
          auto* style_tag = dynamic_cast<StyleTag*>(current_tag);
          auto& property = *style_tag->property(StyleTag::STYLE_REFERENCE_PROPERTY_KEY);
          using ref_prop_cmd_t = PropertiesCommand<ReferenceProperty>;
          scene.submit<ref_prop_cmd_t>(std::set{&property}, *styles.begin());
        }
      }
    }
  }
  return false;
}

bool ObjectTree::canDropMimeData(const QMimeData* data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex& parent) const
{
  const auto actual_column = column >= 0 ? column : parent.column();
  if (ItemModelAdapter::canDropMimeData(data, action, row, column, parent)) {
    return true;
  } else {
    const auto* pdata = qobject_cast<const PropertyOwnerMimeData*>(data);
    if (pdata != nullptr) {
      const auto tags = pdata->tags();
      if (!tags.empty()) {
        switch (actual_column) {
        case OBJECT_COLUMN:
          return true;
        case TAGS_COLUMN:
          return tags.front() != current_tag_predecessor || action != Qt::MoveAction;
        }
        return false;
      }
      if (const auto styles = pdata->styles(); !styles.empty()) {
        if (parent.column() == OBJECT_COLUMN && column < 0) {
          return true;
        } else if (parent.column() == TAGS_COLUMN && column < 0 && styles.size() == 1) {
          if (current_tag != nullptr && current_tag->type() == StyleTag::TYPE) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

std::size_t ObjectTree::max_number_of_tags_on_object() const
{
  const auto objects = scene.object_tree().items();
  const auto cmp
      = [](const Object* lhs, const Object* rhs) { return lhs->tags.size() < rhs->tags.size(); };
  const auto max = std::max_element(objects.begin(), objects.end(), cmp);
  if (max != objects.end()) {
    return (*max)->tags.size();
  } else {
    return 0;
  }
}

std::size_t ObjectTree::insert_position(const Object* predecessor) const
{
  if (predecessor == nullptr) {
    return 0;
  } else {
    return predecessor->position() + 1;
  }
}

}  // namespace omm
