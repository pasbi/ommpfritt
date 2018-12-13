#pragma once

#include <QItemSelectionModel>
#include <set>

namespace omm
{

class ObjectTreeAdapter;
class Tag;

class ObjectTreeSelectionModel : public QItemSelectionModel
{
public:
  explicit ObjectTreeSelectionModel(ObjectTreeAdapter& model);

  bool is_selected(const QModelIndex& index, Tag& tag) const;
  void clear_selection();

  void select(const QModelIndex& index, Tag& tag, QItemSelectionModel::SelectionFlags command);
  void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
  void select( const QItemSelection &selection,
               QItemSelectionModel::SelectionFlags command) override;
  std::set<Tag*> selected_tags() const;


private:
  std::map<QModelIndex, std::set<Tag*>> m_selected_tags;
  using QItemSelectionModel::clearSelection;
};

}  // namespace omm
