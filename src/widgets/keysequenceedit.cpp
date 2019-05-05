#include "widgets/keysequenceedit.h"
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <memory>
#include <QFocusEvent>

namespace omm
{

KeySequenceEdit::KeySequenceEdit(QWidget *parent) : QWidget(parent)
{
  auto layout = std::make_unique<QHBoxLayout>();
  auto reset_button = std::make_unique<QPushButton>(tr("R"));
  reset_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  auto clear_button = std::make_unique<QPushButton>(tr("X"));
  clear_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  auto key_sequence_edit = std::make_unique<QKeySequenceEdit>();
  key_sequence_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  m_key_sequence_edit = key_sequence_edit.get();

  connect(reset_button.get(), &QPushButton::clicked, [this]() {
    m_key_sequence_edit->setKeySequence(m_default_key_sequence);
  });

  connect(clear_button.get(), &QPushButton::clicked, [this]() {
    m_key_sequence_edit->setKeySequence(QKeySequence());
  });

  layout->addWidget(key_sequence_edit.release());
  layout->addWidget(clear_button.release());
  layout->addWidget(reset_button.release());
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());
}

void KeySequenceEdit::set_key_sequence(const QKeySequence &key_sequence)
{
  m_key_sequence_edit->setKeySequence(key_sequence);
}

void KeySequenceEdit::set_default_key_sequence(const QKeySequence &key_sequence)
{
  m_default_key_sequence = key_sequence;
}

QKeySequence KeySequenceEdit::key_sequence() const
{
  return m_key_sequence_edit->keySequence();
}

void KeySequenceEdit::focusInEvent(QFocusEvent *event)
{
  m_key_sequence_edit->setFocus(event->reason());
}



}  // namespace omm
