#pragma once

#include <QFileDialog>
#include <QWidget>

class QLineEdit;
class QPushButton;

namespace omm
{
class FilePathEdit : public QWidget
{
  Q_OBJECT
public:
  explicit FilePathEdit(QWidget* parent = nullptr);
  void set_path(const QString& path);
  [[nodiscard]] QString path() const;
  QFileDialog::FileMode mode = QFileDialog::AnyFile;
  QFileDialog::Options options{};
  void clear();
  void set_placeholder_text(const QString& text);
  QLineEdit& line_edit() const;

Q_SIGNALS:
  void path_changed(const QString& path);

private:
  QString m_path;
  QPushButton* m_button;
  QLineEdit* m_line_edit;
};

}  // namespace omm
