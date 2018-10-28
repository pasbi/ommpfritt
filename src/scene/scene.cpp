#include "scene.h"
#include <random>
#include <glog/logging.h>
#include <assert.h>
#include <QDebug>

#include "object.h"
#include "external/json.hpp"

namespace omm
{

Scene* Scene::m_current = nullptr;

Scene::Scene()
  : m_root(std::make_unique<RootObject>(*this))
{
  m_current = this;
}

Scene::~Scene()
{
  if (m_current == this) {
    m_current = nullptr;
  }
}

Scene::RootObject& Scene::root()
{
  return *m_root;
}

const Scene::RootObject& Scene::root() const
{
  return *m_root;
}

ObjectView Scene::root_view()
{
  return ObjectView(*m_root);
}

Scene* Scene::currentInstance()
{
  return m_current;
}

void Scene::reset()
{
  // ...
}

bool Scene::load(const nlohmann::json& data)
{
  return true;
  //m_root = ...
}

nlohmann::json Scene::save() const
{
  m_root->update_ids();
  return {
    { "root", m_root->to_json() }
  };
}

QModelIndex Scene::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  const auto& parent_item = object_at(parent);
  return createIndex(row, column, &parent_item.child(row));
}

QModelIndex Scene::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  const Object& parent_item = object_at(index);
  if (parent_item.is_root()) {
    return QModelIndex();
  } else {
    return index_of(parent_item.parent());
  }
}

int Scene::rowCount(const QModelIndex& parent) const
{
  return object_at(parent).n_children();
}

int Scene::columnCount(const QModelIndex& parent) const
{
  return 3;
}

QVariant Scene::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  } else if (role != Qt::DisplayRole) {
    return QVariant();
  } else {
    return QString("abc %1 %2").arg(index.row()).arg(index.column());
  }
}

Object& Scene::object_at(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return *m_root;
  } else {
    return *static_cast<Object*>(index.internalPointer());
  }
}

QModelIndex Scene::index_of(Object& object) const
{
  static const auto get_row = [](const Object& object) -> size_t
  {
    assert (!object.is_root());

    const std::vector<std::reference_wrapper<Object>> siblings = object.parent().children();
    for (size_t i = 0; i < siblings.size(); ++i) {
      if (&siblings[i].get() == &object) {
        return i;
      }
    }

    assert(false);
  };

  if (object.is_root()) {
    return QModelIndex();
  } else {
    return createIndex(get_row(object), 0, &object);
  }
}

void Scene::insert_object(std::unique_ptr<Object> object, Object& parent)
{
  size_t n = parent.children().size();
  beginInsertRows(index_of(parent), n, n);
  parent.adopt(std::move(object));
  endInsertRows();
}

QVariant Scene::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags Scene::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return 0;
  } else {
    return QAbstractItemModel::flags(index);
  }
}

}  // namespace omm
