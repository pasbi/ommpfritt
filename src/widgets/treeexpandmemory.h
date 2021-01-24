#pragma once

#include "common.h"
#include <QObject>
#include <map>

class QTreeView;

namespace omm
{
class TreeExpandMemory : public QObject
{
  Q_OBJECT
public:
  using IndexMapper = std::function<QModelIndex(const QModelIndex&)>;
  explicit TreeExpandMemory(QTreeView& view, const IndexMapper& map_to_source);
  explicit TreeExpandMemory(QTreeView& view);

public:
  void restore_later();

private:
  std::map<const void*, bool> m_expanded_store;
  QTreeView& m_view;
  const IndexMapper m_map_to_source;
};

}  // namespace omm
