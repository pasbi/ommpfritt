#include "widgets/filepathedit.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <memory>
#include <QDebug>

namespace omm
{

FilePathEdit::FilePathEdit(QWidget* parent)
  : QWidget(parent)
{
  m_line_edit = std::make_unique<QLineEdit>(this).release();
  m_button = std::make_unique<QPushButton>(tr("..."), this).release();
  std::make_unique<QHBoxLayout>(this).release();
  layout()->addWidget(m_line_edit);
  layout()->addWidget(m_button);
  connect(m_button, &QPushButton::clicked, [this]() {
    QFileDialog dialog(this);
    dialog.setFileMode(mode);
    dialog.setOptions(options);
    if (m_path.size() == 0) {
      dialog.setDirectory(QDir::homePath());
    } else if (QFileInfo info(QString::fromStdString(m_path)); info.isDir()) {
      dialog.setDirectory(QString::fromStdString(m_path));
    } else {
      dialog.setDirectory(info.dir().path());
    }
    if (QDialog::Accepted == dialog.exec()) {
      const auto paths = dialog.selectedFiles();
      if (paths.size() > 0) { set_path(paths.front().toStdString()); }
    }
  });

  connect(m_line_edit, &QLineEdit::textChanged, [this](const QString& text) {
    Q_EMIT path_changed(text.toStdString());
  });
}

void FilePathEdit::set_path(const std::string& path)
{
  if (m_path != path) {
    m_path = path;
    QSignalBlocker signal_blocker(m_line_edit);
    m_line_edit->setText(QString::fromStdString(m_path));
    Q_EMIT path_changed(m_path);
  }
}

std::string FilePathEdit::path() const { return m_path; }
void FilePathEdit::clear() { set_path(tr("").toStdString()); }
void FilePathEdit::set_placeholder_text(const std::string &text)
{
  m_line_edit->setPlaceholderText(QString::fromStdString(text));
}

}  // namespace
