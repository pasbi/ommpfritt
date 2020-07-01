#pragma once

#include <QStandardItemModel>
#include "external/json_fwd.hpp"

class QItemSelection;
class QToolBar;

namespace omm
{

class ToolBarItemModel : public QStandardItemModel
{
  Q_OBJECT
public:
  Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::LinkAction; }
  nlohmann::json encode(const QModelIndexList& indices) const;
  QString encode_str(const QModelIndexList& indices) const;
  QString encode_str() const;
  nlohmann::json encode() const;
  void add_items(const nlohmann::json& code, int row = 0, const QModelIndex& parent = QModelIndex());
  void add_items(const QString& code, int row = 0, const QModelIndex& parent = QModelIndex());
  void populate(QToolBar& tool_bar) const;
  void reset(const QString& configuration);
  static constexpr auto items_key = "items";

  struct ModeSelector
  {
    static constexpr auto cycle_action_key = "cycle_action";
    static constexpr auto activation_actions_key = "activation_actions";
    ModeSelector(const nlohmann::json& j);
    ModeSelector(const QString& cycle_action, const std::vector<QString>& activation_actions);
    nlohmann::json encode() const;

    const QString cycle_action;  // the name of the action that cycles through the modes
    const std::vector<QString> activation_actions;  // the actions than enable the available modes
  };

  static const std::vector<ModeSelector> mode_selectors;

public Q_SLOTS:
  void remove_selection(const QItemSelection& selection);
  void add_group();
  void add_separator();
  void add_switch(const ModeSelector& mode_selector);

protected:
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                       const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                    const QModelIndex& parent) override;
  QMimeData* mimeData(const QModelIndexList& indices) const override;

private:
  void add_single_item(const nlohmann::json& config);

};

} // namespace omm
