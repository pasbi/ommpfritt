#include "widgets/treeexpandmemory.h"
#include <QTimer>
#include <QTreeView>
#include <set>

namespace
{
void enumerate_indices(const QAbstractItemModel& model,
                       std::set<QModelIndex>& indices,
                       const QModelIndex& root)
{
  indices.insert(root);
  for (int row = 0; row < model.rowCount(root); ++row) {
    enumerate_indices(model, indices, model.index(row, 0, root));
  }
}

std::set<QModelIndex> enumerate_indices(const QAbstractItemModel* model)
{
  std::set<QModelIndex> indices;
  if (model != nullptr) {
    enumerate_indices(*model, indices, QModelIndex());
  }
  return indices;
}

}  // namespace

namespace omm
{
TreeExpandMemory::TreeExpandMemory(QTreeView& view, const IndexMapper& map_to_source)
    : m_view(view), m_map_to_source(map_to_source)
{
  connect(&view, &QTreeView::expanded, this, [this](const QModelIndex& index) {
    m_expanded_store[m_map_to_source(index).internalPointer()] = true;
  });
  connect(&view, &QTreeView::collapsed, this, [this](const QModelIndex& index) {
    m_expanded_store[m_map_to_source(index).internalPointer()] = false;
  });
}

TreeExpandMemory::TreeExpandMemory(QTreeView& view)
    : TreeExpandMemory(view, [](const QModelIndex& i) { return i; })
{
}

void TreeExpandMemory::restore_later()
{
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(1, this, [this]() {
    for (auto&& index : enumerate_indices(m_view.model())) {
      const QModelIndex sindex = m_map_to_source(index);
      const auto it = m_expanded_store.find(sindex.internalPointer());
      if (it != m_expanded_store.end()) {
        m_view.setExpanded(index, it->second);
      }
    }
  });
}

}  // namespace omm
