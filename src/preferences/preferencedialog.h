#pragma once

#include <memory>
#include <QDialog>
#include <QTreeWidget>

namespace Ui { class PreferenceDialog; }

namespace omm
{

class PreferenceDialog : public QDialog
{
public:
  explicit PreferenceDialog();
  ~PreferenceDialog();
private:
  std::unique_ptr<Ui::PreferenceDialog> m_ui;
  QTreeWidgetItem* register_preference_page(QTreeWidgetItem* parent, const QString& label,
                                            std::unique_ptr<QWidget> page);

  std::map<QTreeWidgetItem*, int> m_page_map;
};

}  // namespace omm
