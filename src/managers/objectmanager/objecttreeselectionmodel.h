#pragma once

#include <QItemSelectionModel>
#include <set>

namespace omm
{
class ObjectTree;
class Tag;
class Scene;
class AbstractPropertyOwner;

class ObjectTreeSelectionModel : public QItemSelectionModel
{
public:
  explicit ObjectTreeSelectionModel(ObjectTree& adapter);

  bool is_selected(Tag& tag) const;
  void clear_selection();

  void select(Tag& tag, QItemSelectionModel::SelectionFlags command);
  void select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command) override;
  void select(const QItemSelection& selection,
              QItemSelectionModel::SelectionFlags command) override;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);
  void extend_selection(Tag& tag);
  [[nodiscard]] std::set<Tag*> selected_tags() const;
  std::vector<Tag*> selected_tags_ordered(Scene& scene) const;
  [[nodiscard]] const ObjectTree& model() const;

private:
  std::set<Tag*> m_selected_tags;
  Tag* m_current_tag{};
  using QItemSelectionModel::clearSelection;
};

}  // namespace omm
