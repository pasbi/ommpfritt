#include "managers/stylemanager/stylelistadapter.h"
#include "renderers/style.h"

namespace omm
{

StyleListAdapter::StyleListAdapter(Scene& scene)
  : m_scene(scene)
{
  m_scene.Observed<AbstractStyleListObserver>::register_observer(*this);
}

StyleListAdapter::~StyleListAdapter()
{
  m_scene.Observed<AbstractStyleListObserver>::unregister_observer(*this);
}

int StyleListAdapter::rowCount(const QModelIndex& parent) const
{
  assert(!parent.isValid());
  return m_scene.styles().size();
}

QVariant StyleListAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  assert(!index.parent().isValid());

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return QString::fromStdString(m_scene.style(index.row()).name());
  }
  return QVariant();
}

Qt::ItemFlags StyleListAdapter::flags(const QModelIndex &index) const
{
  assert(!index.parent().isValid());
  return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}


Scene& StyleListAdapter::scene() const
{
  return m_scene;
}

void StyleListAdapter::beginInsertStyle(int row)
{
  beginInsertRows(QModelIndex(), row, row);
}

void StyleListAdapter::endInsertStyle()
{
  endInsertRows();
}

void StyleListAdapter::beginResetStyles()
{
  beginResetModel();
}

void StyleListAdapter::endResetStyles()
{
  endResetModel();
}

void StyleListAdapter::beginRemoveStyle(int row)
{
  beginRemoveRows(QModelIndex(), row, row);
}

void StyleListAdapter::endRemoveStyle()
{
  endRemoveRows();
}

bool StyleListAdapter::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  assert(index.column() == 0);
  assert(!index.parent().isValid());

  auto& style = m_scene.style(index.row());
  auto& name_property = style.property<StringProperty>(Object::NAME_PROPERTY_KEY);
  name_property.set_value(value.toString().toStdString());
  return true;
}

}  // namespace omm
