#pragma once

#include <QKeySequence>
#include <QSortFilterProxyModel>

namespace omm
{
class KeyBindings;

class KeyBindingsProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  KeyBindingsProxyModel(KeyBindings& key_bindings);
  [[nodiscard]] bool filterAcceptsRow(int source_row,
                                      const QModelIndex& source_parent) const override;

public:
  /**
   * @brief set_action_filter the model will keep all actions where `action_name` is a part of
   *  the translated action label or key-sequence.
   * @param action_name the string to look for
   */
  void set_action_name_filter(const QString& action_name);
  void set_action_sequence_filter(const QKeySequence& sequence);

private:
  QString m_action_name_filter;
  QKeySequence m_action_sequence_filter;
};

}  // namespace omm
