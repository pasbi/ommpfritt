#include "preferences/keysequenceedit.h"
#include "mainwindow/iconprovider.h"
#include "syncpalettedecorator.h"
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QStyle>
#include <memory>

namespace omm
{
KeySequenceEdit::KeySequenceEdit(QWidget* parent) : QWidget(parent), m_ui(new Ui::KeySequenceEdit)

{
  m_ui->setupUi(this);
  m_ui->pb_clear->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));
  m_ui->pb_reset->setIcon(IconProvider::pixmap("revert"));

  connect(m_ui->pb_reset, &QPushButton::clicked, [this]() {
    m_ui->key_sequence_edit->setKeySequence(m_default_key_sequence);
  });

  connect(m_ui->pb_clear, &QPushButton::clicked, [this]() {
    m_ui->key_sequence_edit->setKeySequence(QKeySequence());
  });

  SyncPaletteButtonDecorator::decorate(*m_ui->pb_clear);
  SyncPaletteButtonDecorator::decorate(*m_ui->pb_reset);
}

void KeySequenceEdit::set_key_sequence(const QKeySequence& key_sequence)
{
  m_ui->key_sequence_edit->setKeySequence(key_sequence);
}

void KeySequenceEdit::set_default_key_sequence(const QKeySequence& key_sequence)
{
  m_default_key_sequence = key_sequence;
}

QKeySequence KeySequenceEdit::key_sequence() const
{
  return m_ui->key_sequence_edit->keySequence();
}

void KeySequenceEdit::focusInEvent(QFocusEvent* event)
{
  m_ui->key_sequence_edit->setFocus(event->reason());
}

bool KeySequenceEdit::focusNextPrevChild(bool next)
{
  // this makes it possible to use `Tab` in a user-defined key sequence.
  Q_UNUSED(next)
  return false;
}

}  // namespace omm
