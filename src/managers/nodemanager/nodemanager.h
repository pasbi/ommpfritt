#pragma once

#include "managers/manager.h"

namespace Ui
{
class NodeManager;
}

namespace omm
{
class AbstractPropertyOwner;
class NodeModel;
class NodeManager : public Manager
{
  Q_OBJECT
public:
  explicit NodeManager(Scene& scene);
  ~NodeManager() override;
  NodeManager(NodeManager&&) = delete;
  NodeManager(const NodeManager&) = delete;
  NodeManager& operator=(NodeManager&&) = delete;
  NodeManager& operator=(const NodeManager&) = delete;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "NodeManager");
  [[nodiscard]] QString type() const override;
  bool perform_action(const QString& name) override;

  void set_model(NodeModel* model);

public:
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);

private:
  std::unique_ptr<Ui::NodeManager> m_ui;
  std::unique_ptr<QMenu> make_add_nodes_menu(KeyBindings& kb);
  std::unique_ptr<QMenu> make_context_menu();
};

}  // namespace omm
