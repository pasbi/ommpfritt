#pragma once

#include <QItemSelectionModel>
#include <set>

namespace omm
{

class ObjectTreeAdapter;
class Tag;
class Scene;

class ObjectTreeSelectionModel : public QItemSelectionModel
{
public:
  explicit ObjectTreeSelectionModel(ObjectTreeAdapter& model);

  bool is_selected(Tag& tag) const;
  void clear_selection();

  void select(Tag& tag, QItemSelectionModel::SelectionFlags command);
  void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
  void select( const QItemSelection &selection,
               QItemSelectionModel::SelectionFlags command) override;
  void extend_selection(Tag& tag);
  std::set<Tag*> selected_tags() const;
  std::vector<Tag*> selected_tags_ordered(Scene& scene) const;


private:
  std::set<Tag*> m_selected_tags;
  Tag* m_current_tag;
  using QItemSelectionModel::clearSelection;
};

}  // namespace omm
