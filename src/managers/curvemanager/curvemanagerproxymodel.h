#include <QSortFilterProxyModel>

namespace omm
{

class Animator;

class CurveManagerProxyModel : public QSortFilterProxyModel
{
public:
  explicit CurveManagerProxyModel(Animator& animator);
  [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
  int columnCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& proxyIndex, int role) const override;

private:
  Animator& m_animator;
};

}  // namespace omm
