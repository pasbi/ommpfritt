#pragma once

#include <QAbstractItemModel>
#include "scene/scene.h"

class QItemSelection;

namespace omm
{

class Scene;
class Object;

class ObjectTreeAdapter : public QAbstractItemModel, public AbstractObjectTreeObserver
{
  Q_OBJECT

public:
  using RootObject = Object;
  explicit ObjectTreeAdapter(Object& root_object);
  ~ObjectTreeAdapter();

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::DropActions supportedDragActions() const override;
  bool canDropMimeData( const QMimeData *data, Qt::DropAction action,
                        int row, int column, const QModelIndex &parent ) const override;
  bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                     int row, int column, const QModelIndex &parent ) override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList &indexes) const override;

  Object& object_at(const QModelIndex& index) const;

  QModelIndex index_of(Object& object) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  void beginInsertObjects(Object& parent, int start, int end) override;
  void endInsertObjects() override;
  void beginMoveObject(const ObjectTreeContext& context) override;
  void endMoveObject() override;
  Scene& scene() const;

private:
  Object& m_root;

  std::vector<omm::ObjectTreeContext>
  make_new_contextes(const QMimeData* data, int row, const QModelIndex& parent) const;
};

}  // namespace omm