#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <memory>

namespace Ui
{
class PreferenceDialog;
}

namespace omm
{
class PreferencePage;
class PreferenceDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PreferenceDialog();
  ~PreferenceDialog() override;
  PreferenceDialog(PreferenceDialog&&) = delete;
  PreferenceDialog(const PreferenceDialog&) = delete;
  PreferenceDialog& operator=(PreferenceDialog&&) = delete;
  PreferenceDialog& operator=(const PreferenceDialog&) = delete;

public:
  void accept() override;
  void reject() override;

private:
  std::unique_ptr<Ui::PreferenceDialog> m_ui;
  QTreeWidgetItem* register_preference_page(QTreeWidgetItem* parent,
                                            const QString& label,
                                            std::unique_ptr<PreferencePage> page);

  std::map<QTreeWidgetItem*, PreferencePage*> m_page_map;
};

}  // namespace omm
