#pragma once

#include <QWidget>

class QAbstractItemModel;
class QComboBox;

namespace omm
{
/**
 * @brief The ComboBox class
 * This class is supposed to serve as view into QAbstractItemModel.
 * Note that the default QComboBox also supports QAbstractItemModel as a data source.
 * However, if the item at current index is removed, QComboBox sets the index to the closest valid
 * index.
 * The ComboBox class, however, sets the current index to invlaid (-1) in that case.
 * Only the required methods are implemented, but the underlying QComboBox can be accessed read
 * only.
 */
class ComboBox : public QWidget
{
  Q_OBJECT
public:
  explicit ComboBox(QWidget* widget = nullptr);
  void set_model(QAbstractItemModel& model);
  [[nodiscard]] const QComboBox* view() const
  {
    return m_combo_box;
  }

public:
  void set_current_index(int index);
  void set_current_text(const QString& text);

Q_SIGNALS:
  void current_index_changed(int index);

private:
  QAbstractItemModel* m_model = nullptr;
  QComboBox* m_combo_box;

private:
  void reset();
  void insert(const QModelIndex& index, int first, int last);
  void remove(const QModelIndex& index, int first, int last);
  void change(const QModelIndex& tl, const QModelIndex& br, const QVector<int>& roles);

private:
  static constexpr std::initializer_list<int> item_roles{Qt::BackgroundRole,
                                                         Qt::ForegroundRole,
                                                         Qt::DisplayRole};
  void set_item(std::size_t i, bool add, const QVector<int>& roles = QVector<int>(item_roles));
};

}  // namespace omm
