#include "managers/stylemanager/stylelistadapter.h"

namespace omm
{

StyleListAdapter::StyleListAdapter(StylePool& style_pool)
  : m_style_pool(style_pool)
{
  m_style_pool.Observed<AbstractStyleListObserver>::register_observer(*this);
}

StyleListAdapter::~StyleListAdapter()
{
  m_style_pool.Observed<AbstractStyleListObserver>::unregister_observer(*this);
}

int StyleListAdapter::rowCount(const QModelIndex& parent) const
{
  return m_style_pool.styles().size();
}

QVariant StyleListAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return QString::fromStdString(m_style_pool.styles()[index.row()]->name());
  }
  return QVariant();
}

StylePool& StyleListAdapter::style_pool() const
{
  return m_style_pool;
}

void StyleListAdapter::beginInsertObject(int row)
{
  beginInsertRows(QModelIndex(), row, row);
}

void StyleListAdapter::endInsertObject()
{
  endInsertRows();
}

}  // namespace omm
