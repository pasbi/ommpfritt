#pragma once

#include "external/json_fwd.hpp"
#include <QStandardItemModel>

class QItemSelection;
class QToolBar;

namespace omm
{
class ToolBarItemModel : public QStandardItemModel
{
  Q_OBJECT
public:
  [[nodiscard]] Qt::DropActions supportedDropActions() const override
  {
    return Qt::MoveAction | Qt::LinkAction;
  }
  [[nodiscard]] nlohmann::json encode(const QModelIndexList& indices) const;
  [[nodiscard]] QString encode_str(const QModelIndexList& indices) const;
  [[nodiscard]] QString encode_str() const;
  [[nodiscard]] nlohmann::json encode() const;
  void
  add_items(const nlohmann::json& code, int row = 0, const QModelIndex& parent = QModelIndex());
  void add_items(const QString& code, int row = 0, const QModelIndex& parent = QModelIndex());
  void populate(QToolBar& tool_bar) const;
  void reset(const QString& configuration);
  static constexpr auto items_key = "items";

  class BadConfigurationError : public std::runtime_error
  {
  public:
    BadConfigurationError(const QString& description) noexcept;
  };

public:
  void remove_selection(const QItemSelection& selection);
  void add_group();
  void add_separator();
  void add_mode_selector(const QString& mode_selector_name);

protected:
  bool canDropMimeData(const QMimeData* data,
                       Qt::DropAction action,
                       int row,
                       int column,
                       const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data,
                    Qt::DropAction action,
                    int row,
                    int column,
                    const QModelIndex& parent) override;
  [[nodiscard]] QMimeData* mimeData(const QModelIndexList& indices) const override;

private:
  void add_single_item(const nlohmann::json& config);
};

}  // namespace omm
