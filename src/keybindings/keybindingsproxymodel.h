#pragma once

#include <QSortFilterProxyModel>

namespace omm
{

class KeyBindings;

class KeyBindingsProxyModel : public QSortFilterProxyModel
{
public:
  KeyBindingsProxyModel(KeyBindings& key_bindings);
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

  /**
   * @brief set_action_filter the model will keep all actions where `action_name` is a part of
   *  the translated action label or key-sequence.
   * @param action_name the string to look for
   */
  void set_action_name_filter(const std::string& action_name);
  void set_action_sequence_filter(const std::string& action_sequence);

private:
  std::string m_action_name_filter;
  std::string m_action_sequence_filter;
};

}  // namespace omm
