#pragma once

#include <QKeySequence>
#include <QWidget>

class QKeySequenceEdit;

namespace omm
{

class KeySequenceEdit : public QWidget
{
  Q_OBJECT
public:
  explicit KeySequenceEdit(QWidget* parent = nullptr);
  void set_key_sequence(const QKeySequence& key_sequence);
  void set_default_key_sequence(const QKeySequence& key_sequence);
  QKeySequence key_sequence() const;
  void focusInEvent(QFocusEvent *event) override;

private:
  QKeySequenceEdit* m_key_sequence_edit;
  QKeySequence m_default_key_sequence;
};

}  // namespace omm
