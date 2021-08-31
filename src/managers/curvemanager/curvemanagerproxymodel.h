#include <QSortFilterProxyModel>

namespace omm
{

class Animator;

class CurveManagerProxyModel : public QSortFilterProxyModel
{
public:
  explicit CurveManagerProxyModel(Animator& animator);
  [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
  [[nodiscard]] int columnCount(const QModelIndex&) const override;
  [[nodiscard]] QVariant data(const QModelIndex& proxyIndex, int role) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;
  [[nodiscard]] QModelIndex mapToSource(const QModelIndex &index) const override;
  [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;
  [[nodiscard]] QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

private:
  Animator& m_animator;
};

}  // namespace omm
