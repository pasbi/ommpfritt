#include "widgets/combobox.h"
#include <QAbstractItemModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <memory>

namespace omm
{
ComboBox::ComboBox(QWidget* widget) : QWidget(widget)
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto cb = std::make_unique<QComboBox>();
  m_combo_box = cb.get();
  layout->addWidget(cb.release());
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());
  connect(m_combo_box,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &ComboBox::current_index_changed);
}

void ComboBox::set_model(QAbstractItemModel& model)
{
  m_model = &model;
  connect(&model, &QAbstractItemModel::modelReset, this, &ComboBox::reset);
  connect(&model, &QAbstractItemModel::rowsInserted, this, &ComboBox::insert);
  connect(&model, &QAbstractItemModel::rowsRemoved, this, &ComboBox::remove);
  connect(&model, &QAbstractItemModel::dataChanged, this, &ComboBox::change);
  reset();
}

void ComboBox::set_current_text(const QString& text)
{
  m_combo_box->setCurrentText(text);
}

void ComboBox::set_current_index(int index)
{
  m_combo_box->setCurrentIndex(index);
}

void ComboBox::reset()
{
  m_combo_box->clear();

  const std::size_t n = m_model->rowCount();
  for (std::size_t i = 0; i < n; ++i) {
    set_item(i, true);
  }
}

void ComboBox::insert([[maybe_unused]] const QModelIndex& index, int first, int last)
{
  assert(!index.isValid());
  for (int i = first; i <= last; ++i) {
    set_item(i, true);
  }
}

void ComboBox::remove([[maybe_unused]] const QModelIndex& index, int first, int last)
{
  assert(!index.isValid());
  int current_index = m_combo_box->currentIndex();
  if (current_index < 0) {
    // if the current index is invalid, keep it.
  } else if (current_index >= first && current_index <= last) {
    // if the current index is valid and inside the deleted range, make it invalid.
    current_index = -1;
  } else if (current_index < first) {
    // if the current index is before the deleted range, keep it.
  } else if (current_index > last) {
    // if the current index is behind the deleted range, shift it.
    const int n_removed = last - first + 1;
    current_index -= n_removed;
  }

  {
    QSignalBlocker blocker(this);
    for (int i = last; i >= first; --i) {
      m_combo_box->removeItem(i);
    }
  }

  m_combo_box->setCurrentIndex(current_index);
  //  Q_EMIT current_index_changed(current_index);
}

void ComboBox::change(const QModelIndex& tl, const QModelIndex& br, const QVector<int>& roles)
{
  for (int i = tl.row(); i <= br.row(); ++i) {
    set_item(i, false, roles);
  }
}

void ComboBox::set_item(std::size_t i, bool add, const QVector<int>& roles)
{
  const QModelIndex index = m_model->index(i, 0);
  if (add) {
    m_combo_box->insertItem(i, "");
  }
  for (auto role : roles) {
    m_combo_box->setItemData(i, m_model->data(index, role), role);
  }
}

}  // namespace omm
