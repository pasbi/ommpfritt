#include "widgets/filepathedit.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <memory>

namespace omm
{
FilePathEdit::FilePathEdit(QWidget* parent) : QWidget(parent)
{
  static constexpr int BUTTON_WIDTH = 40;

  setLayout(std::make_unique<QHBoxLayout>().release());

  auto line_edit = std::make_unique<QLineEdit>(this);
  m_line_edit = line_edit.get();
  layout()->addWidget(line_edit.release());

  auto button = std::make_unique<QPushButton>(tr("..."), this);
  button->setMaximumWidth(BUTTON_WIDTH);
  m_button = button.get();
  layout()->addWidget(button.release());

  connect(m_button, &QPushButton::clicked, [this]() {
    QFileDialog dialog(this);
    dialog.setFileMode(mode);
    dialog.setOptions(options);
    if (m_path.size() == 0) {
      dialog.setDirectory(QDir::homePath());
    } else if (QFileInfo info(m_path); info.isDir()) {
      dialog.setDirectory(m_path);
    } else {
      dialog.setDirectory(info.dir().path());
    }
    if (QDialog::Accepted == dialog.exec()) {
      const auto paths = dialog.selectedFiles();
      if (!paths.empty()) {
        set_path(paths.front());
      }
    }
  });

  connect(m_line_edit, &QLineEdit::textChanged, [this](const QString& text) {
    m_path = text;
    Q_EMIT path_changed(text);
  });

  layout()->setContentsMargins(0, 0, 0, 0);
}

void FilePathEdit::set_path(const QString& path)
{
  if (m_path != path) {
    m_path = path;
    QSignalBlocker signal_blocker(m_line_edit);
    m_line_edit->setText(m_path);
    Q_EMIT path_changed(m_path);
  }
}

QString FilePathEdit::path() const
{
  return m_path;
}
void FilePathEdit::clear()
{
  set_path(tr(""));
}
void FilePathEdit::set_placeholder_text(const QString& text)
{
  m_line_edit->setPlaceholderText(text);
}

QLineEdit& FilePathEdit::line_edit() const
{
  return *m_line_edit;
}

}  // namespace omm
