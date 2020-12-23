#pragma once

#include "scene/itemmodeladapter.h"

namespace omm
{
class Object;

class ObjectTree
    : public ItemModelAdapter<ObjectTree, Object, QAbstractItemModel>
    , public Structure<Object>
{
  Q_OBJECT
public:
  constexpr static bool is_tree = true;

  ObjectTree(std::unique_ptr<Object> root, Scene& scene);

public:
  virtual void insert(ObjectTreeOwningContext& context);
  void move(TreeMoveContext<Object>& context);
  void remove(ObjectTreeOwningContext& context);
  bool can_move_object(const TreeMoveContext<Object>& new_context) const;
  std::unique_ptr<Object> replace_root(std::unique_ptr<Object> new_root);
  Object& root() const;
  bool contains(const Object& t) const;

  std::size_t insert_position(const Object* predecessor) const override;
  std::set<Object*> items() const override;
  std::size_t position(const Object& item) const override;
  const Object* predecessor(const Object& sibling) const override;
  using Structure<Object>::predecessor;
  std::unique_ptr<Object> remove(Object& t) override;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Object& item_at(const QModelIndex& index) const override;
  QModelIndex index_of(Object& object) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  bool dropMimeData(const QMimeData* data,
                    Qt::DropAction action,
                    int row,
                    int column,
                    const QModelIndex& parent) override;

  bool canDropMimeData(const QMimeData* data,
                       Qt::DropAction action,
                       int row,
                       int column,
                       const QModelIndex& parent) const override;

  constexpr static int TAGS_COLUMN = 2;
  constexpr static int VISIBILITY_COLUMN = 1;
  constexpr static int OBJECT_COLUMN = 0;

  std::size_t max_number_of_tags_on_object() const;

public:
  Tag* current_tag_predecessor = nullptr;
  Tag* current_tag = nullptr;

private:
  std::unique_ptr<Object> m_root;

  mutable bool m_item_cache_is_dirty = true;
  mutable std::set<Object*> m_item_cache;
  Scene& m_scene;

Q_SIGNALS:
  void expand_item(const QModelIndex&);
};

}  // namespace omm
