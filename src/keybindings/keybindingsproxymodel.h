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
   * @brief set_context_filter the model will keep all actions in context named `context_name`.
   * @param context_name the context name to keep. Set to empty to accept any context.
   */
  void set_context_filter(const std::string& context_name);

  /**
   * @brief set_action_filter the model will keep all actions where `action_name` is a part of
   *  the translated action label or key-sequence.
   * @param action_name the string to look for
   */
  void set_action_filter(const std::string& action_name);

  std::vector<std::string> contextes() const;

private:
  std::string m_context_name_filter;
  std::string m_action_filter;
};

}  // namespace omm
