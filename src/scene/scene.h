#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include <QAbstractItemModel>


namespace omm
{

class Object;

class Scene : public QAbstractItemModel
{
  Q_OBJECT

public:
  using RootObject = Object;
  Scene();
  ~Scene();

  RootObject& root();
  const RootObject& root() const;
  ObjectView root_view();

  static Scene* currentInstance();

  void reset();
  bool load(const nlohmann::json& data);
  nlohmann::json save() const;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Object& object_at(const QModelIndex& index) const;

  QModelIndex index_of(Object& object) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;


  void insert_object(std::unique_ptr<Object> object, Object& parent);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;


private:
  std::unique_ptr<RootObject> m_root;
  static Scene* m_current;
};

}  // namespace omm

