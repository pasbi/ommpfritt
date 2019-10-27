#pragma once

#include <QWidget>
#include <QFileDialog>

class QLineEdit;
class QPushButton;

namespace omm
{

class FilePathEdit : public QWidget
{
  Q_OBJECT
public:
  explicit FilePathEdit(QWidget* parent);
  void set_path(const QString& path);
  QString path() const;
  QFileDialog::FileMode mode = QFileDialog::AnyFile;
  QFileDialog::Options options = 0;
  void clear();
  void set_placeholder_text(const QString& text);

Q_SIGNALS:
  void path_changed(const QString& path);

private:
  QString m_path;
  QPushButton* m_button;
  QLineEdit* m_line_edit;
};

}  // namespace
