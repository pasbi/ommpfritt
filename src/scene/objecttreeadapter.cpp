#include "scene/objecttreeadapter.h"

#include <QItemSelection>
#include <QDebug>
#include <glog/logging.h>

#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "common.h"
#include "commands/movecommand.h"
#include "commands/copycommand.h"
#include "commands/addcommand.h"
#include "commands/movetagscommand.h"
#include "properties/stringproperty.h"
#include "scene/scene.h"
#include "abstractraiiguard.h"
#include "tags/tag.h"
#include "tags/styletag.h"
#include "scene/contextes.h"
namespace
{

using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;

class AbstractRAIISceneInvalidatorGuard : public AbstractRAIIGuard
{
protected:
  AbstractRAIISceneInvalidatorGuard(omm::Scene& scene) : m_scene(scene) {}
  ~AbstractRAIISceneInvalidatorGuard() override { m_scene.invalidate(); }

private:
  omm::Scene& m_scene;
};

template<typename T>
T* last(const std::vector<T*>& ts) { return ts.size() == 0 ? nullptr : ts.back(); }

void drop_tags_onto_object( omm::Scene& scene, omm::Object& object,
                            const std::vector<omm::Tag*>& tags, Qt::DropAction action )
{
  if (tags.size() > 0) {
    switch (action) {
    case Qt::CopyAction:
      scene.undo_stack.beginMacro("copy tags");
      for (auto* tag : tags) {
        auto tag_clone = tag->clone();
        tag_clone->owner = &object;
        scene.submit<AddTagCommand>(object.tags, std::move(tag_clone));
      }
      scene.undo_stack.endMacro();
      break;
    case Qt::MoveAction:
      scene.submit<omm::MoveTagsCommand>(tags, object, last(object.tags.ordered_items()));
      break;
    default: break;
    }
  }
}

void drop_tags_behind( omm::Scene& scene, omm::Object& object, omm::Tag* current_tag,
                        const std::vector<omm::Tag*>& tags, Qt::DropAction action )
{
  if (current_tag != nullptr && current_tag->owner != &object) {
    const auto& tags = object.tags;
    current_tag = last(tags.ordered_items());
  }
  if (tags.size() > 0) {
    switch (action) {
    case Qt::CopyAction:
      scene.undo_stack.beginMacro("copy tags");
      for (auto* tag : tags) {
        auto tag_clone = tag->clone();
        tag_clone->owner = &object;
        omm::ListOwningContext<omm::Tag> context(std::move(tag_clone), current_tag);
        scene.submit<AddTagCommand>(object.tags, std::move(context));
      }
      scene.undo_stack.endMacro();
      break;
    case Qt::MoveAction:
      scene.submit<omm::MoveTagsCommand>(tags, object, current_tag);
      break;
    default: break;
    }
  }
}

void drop_style_onto_object( omm::Scene& scene, omm::Object& object,
                             const std::vector<omm::Style*>& styles )
{
  if (styles.size() > 0) {
    scene.undo_stack.beginMacro("set styles tags");
    for (auto* style : styles) {
      auto style_tag = std::make_unique<omm::StyleTag>(object);
      style_tag->property(omm::StyleTag::STYLE_REFERENCE_PROPERTY_KEY).set(style);

      scene.submit<AddTagCommand>(object.tags, std::move(style_tag));
    }
    scene.undo_stack.endMacro();
  }
}

}  // namespace

namespace omm
{

ObjectTreeAdapter::ObjectTreeAdapter(Scene& scene, Tree<Object>& tree)
  : ItemModelAdapter(scene, tree)
{

}

QModelIndex ObjectTreeAdapter::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  return createIndex(row, column, &item_at(parent).child(row));
}

QModelIndex ObjectTreeAdapter::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  const Object& parent_item = item_at(index);
  if (parent_item.is_root()) {
    return QModelIndex();
  } else {
    return index_of(parent_item.parent());
  }
}

int ObjectTreeAdapter::rowCount(const QModelIndex& parent) const
{
  return item_at(parent).n_children();
}

int ObjectTreeAdapter::columnCount(const QModelIndex& parent) const
{
  return 3;
}

bool ObjectTreeAdapter::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  switch (index.column()) {
  case 0:
    item_at(index).property(Object::NAME_PROPERTY_KEY).set(value.toString().toStdString());
    return true;
  }

  return false;
}

QVariant ObjectTreeAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  switch (index.column()) {
  case 0:
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return QString::fromStdString(item_at(index).name());;
    }
  }
  return QVariant();
}

Object& ObjectTreeAdapter::item_at(const QModelIndex& index) const
{
  if (index.isValid()) {
    assert(index.internalPointer() != nullptr);
    return *static_cast<Object*>(index.internalPointer());
  } else {
    return structure.root();
  }
}

QModelIndex ObjectTreeAdapter::index_of(Object& object) const
{
  if (object.is_root()) {
    return QModelIndex();
  } else {
    return createIndex(scene.object_tree.position(object), 0, &object);
  }
}

QVariant ObjectTreeAdapter::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section) {
    case 0: return "object";
    case 1: return "is visible";
    case 2: return "tags";
    }
  }

 return QVariant();
}

Qt::ItemFlags ObjectTreeAdapter::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return Qt::ItemIsDropEnabled;
  }

  switch (index.column()) {
  case 0:
    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  case 2:
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  default:
    return Qt::ItemIsEnabled;
  }
}

std::unique_ptr<AbstractRAIIGuard>
ObjectTreeAdapter::acquire_inserter_guard(Object& parent, int row)
{
  class InserterGuard : public AbstractRAIISceneInvalidatorGuard
  {
  public:
    InserterGuard(ObjectTreeAdapter& model, const QModelIndex& parent, int row)
      : AbstractRAIISceneInvalidatorGuard(model.scene), m_model(model)
    {
      m_model.beginInsertRows(parent, row, row);
    }

    ~InserterGuard() { m_model.endInsertRows(); }
  private:
    ObjectTreeAdapter& m_model;
  };
  return std::make_unique<InserterGuard>(*this, index_of(parent), row);
}

std::unique_ptr<AbstractRAIIGuard>
ObjectTreeAdapter::acquire_mover_guard(const ObjectTreeMoveContext& context)
{
  class MoverGuard : public AbstractRAIISceneInvalidatorGuard
  {
  public:
    MoverGuard( ObjectTreeAdapter& model, const QModelIndex& old_parent, const int old_pos,
                const QModelIndex& new_parent, const int new_pos )
      : AbstractRAIISceneInvalidatorGuard(model.scene), m_model(model)
    {
      m_model.beginMoveRows(old_parent, old_pos, old_pos, new_parent, new_pos);
    }

    ~MoverGuard() { m_model.endMoveRows(); }
  private:
    ObjectTreeAdapter& m_model;
  };

  assert(!context.subject.get().is_root());
  Object& old_parent = context.subject.get().parent();
  Object& new_parent = context.parent.get();
  const auto old_pos = scene.object_tree.position(context.subject);
  const auto new_pos = scene.object_tree.insert_position(context.predecessor);

  if (old_pos == new_pos && &old_parent == &new_parent) {
    return nullptr;
  } else {
    return std::make_unique<MoverGuard>( *this, index_of(old_parent), old_pos,
                                         index_of(new_parent), new_pos );
  }
}

std::unique_ptr<AbstractRAIIGuard>
ObjectTreeAdapter::acquire_remover_guard(const Object& object)
{
  class RemoverGuard : public AbstractRAIISceneInvalidatorGuard
  {
  public:
    RemoverGuard(ObjectTreeAdapter& model, const QModelIndex& parent, int row)
      : AbstractRAIISceneInvalidatorGuard(model.scene), m_model(model)
    {
      m_model.beginRemoveRows(parent, row, row);
    }
    ~RemoverGuard() { m_model.endRemoveRows(); }

  private:
    ObjectTreeAdapter& m_model;
  };
  return std::make_unique<RemoverGuard>( *this, index_of(object.parent()),
                                         scene.object_tree.position(object) );
}

std::unique_ptr<AbstractRAIIGuard>
ObjectTreeAdapter::acquire_reseter_guard()
{
  class ReseterGuard : public AbstractRAIISceneInvalidatorGuard
  {
  public:
    ReseterGuard(ObjectTreeAdapter& model)
      : AbstractRAIISceneInvalidatorGuard(model.scene), m_model(model)
    {
      m_model.beginResetModel();
    }
    ~ReseterGuard() { m_model.endResetModel(); }

  private:
    ObjectTreeAdapter& m_model;
  };
  return std::make_unique<ReseterGuard>(*this);
}

bool ObjectTreeAdapter::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent )
{
  if (ItemModelAdapter::dropMimeData(data, action, row, column, parent)) {
    LOG(INFO) << "done";
    return true;
  } else {
    const auto* pdata = qobject_cast<const PropertyOwnerMimeData*>(data);
    if (const auto tags = pdata->tags(); tags.size() > 0) {
      if (parent.column() == OBJECT_COLUMN) {
        assert(parent.isValid());  // otherwise, column was < 0.
        drop_tags_onto_object(scene, item_at(parent), tags, action);
        return true;
      } else if (parent.column() == TAGS_COLUMN) {
        if (std::find(tags.begin(), tags.end(), current_tag) == tags.end()) {
          drop_tags_behind(scene, item_at(parent), current_tag, tags, action);
          return true;
        } else {
          if (action == Qt::MoveAction) {
            // cannot reorder list of tags relative to a tag which is contained in that very list.
            return false;
          } else {
            return false;
          }
        }
        return true;
      }
    }
    if (const auto styles = pdata->styles(); styles.size() > 0) {
      if (parent.column() == 0) {
        assert(parent.isValid()); // otherwise, column was < 0
        drop_style_onto_object(scene, item_at(parent), styles);
      } else {
        // TODO if parent.column() == 2 and current_tag is a style tag, set style.
      }
    }
  }
  return false;
}

bool ObjectTreeAdapter::canDropMimeData( const QMimeData *data, Qt::DropAction action,
                                         int row, int column, const QModelIndex &parent ) const
{
  if (ItemModelAdapter::canDropMimeData(data, action, row, column, parent)) {
    return true;
  } else {
    const auto* pdata = qobject_cast<const PropertyOwnerMimeData*>(data);
    if (pdata->tags().size() > 0 && (parent.column() == 2 || parent.column() == 0)) {
      return true;
    }
    if (pdata->styles().size() > 0) {
      return parent.column() == OBJECT_COLUMN;
      // TODO also allow drop onto Style tag
    }
  }
  return false;
}


}  // namespace omm
