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

void StyleListAdapter::beginInsertStyles(int row)
{
  beginInsertRows(QModelIndex(), row, row);
}

void StyleListAdapter::endInsertStyles()
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

}  // namespace omm
