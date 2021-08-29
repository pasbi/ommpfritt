#pragma once

#include <QItemDelegate>
#include <QKeySequenceEdit>
#include <QTreeView>
#include <memory>

namespace omm
{
class AbstractPreferencesTreeViewDelegate;
class KeyBindings;

class PreferencesTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit PreferencesTreeView(QWidget* parent = nullptr);
  ~PreferencesTreeView() override;
  PreferencesTreeView(PreferencesTreeView&&) = delete;
  PreferencesTreeView(const PreferencesTreeView&) = delete;
  PreferencesTreeView& operator=(PreferencesTreeView&&) = delete;
  PreferencesTreeView& operator=(const PreferencesTreeView&) = delete;
  void set_model(QAbstractItemModel& model,
                 std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> delegates);
  void transfer_editor_data_to_model();
  void close_all_editors();

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  void update_column_width();

private:
  std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> m_column_delegates;
};

}  // namespace omm
