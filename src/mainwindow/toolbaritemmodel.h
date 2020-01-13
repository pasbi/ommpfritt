#include <QStandardItemModel>

class QItemSelection;

namespace omm
{

class ToolBarItemModel : public QStandardItemModel
{
  Q_OBJECT
public:
  explicit ToolBarItemModel(const QString& code);
  Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::LinkAction; }
  QString encode(int row_begin, int row_end, const QModelIndex& parent) const;
  QString encode() const;
  void decode(const QString& code, int row, const QModelIndex& parent);

public Q_SLOTS:
  void add_button();
  void add_separator();
  void remove_selection(const QItemSelection& selection);

protected:
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
  QMimeData* mimeData(const QModelIndexList& indices) const override;

};

} // namespace omm
