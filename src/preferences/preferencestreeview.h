#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <QKeySequenceEdit>
#include <memory>

namespace omm
{

class AbstractPreferencesTreeViewDelegate;
class KeyBindings;

class PreferencesTreeView: public QTreeView
{
  Q_OBJECT
public:
  explicit PreferencesTreeView(QWidget* parent = nullptr);
  ~PreferencesTreeView();
  void set_model(QAbstractItemModel& key_bindings);
  void transfer_editor_data_to_model();
  virtual void set_delegate(std::unique_ptr<AbstractPreferencesTreeViewDelegate> delegate);

protected:
  void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
  void update_column_width();

private:
  std::unique_ptr<AbstractPreferencesTreeViewDelegate> m_sequence_column_delegate;
};

}  // namespace omm
