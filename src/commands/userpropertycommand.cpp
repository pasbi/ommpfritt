#include "commands/userpropertycommand.h"
#include "commands/nodecommand.h"
#include "managers/nodemanager/node.h"
#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace
{

auto transform(const std::vector<QString>& keys)
{
  using pair_type = std::pair<QString, std::unique_ptr<omm::Property>>;
  return ::transform<pair_type>(keys, [](const QString& key) {
    return pair_type { key, nullptr };
  });
}

}  // namespace

namespace omm
{

UserPropertyCommand
::UserPropertyCommand(const std::vector<QString>& deletions,
                      std::vector<std::pair<QString, std::unique_ptr<Property>>> additions,
                      const std::map<Property*, Property::Configuration>& changes,
                      AbstractPropertyOwner& owner)
  : Command(QObject::tr("User Property"))
  , m_deletions(transform(deletions))
  , m_additions(std::move(additions))
  , m_changes(changes)
  , m_owner(owner)
{
  if (Node* node = kind_cast<Node*>(&m_owner); node != nullptr) {
    for (auto&& [key, property] : m_deletions) {
      InputPort* ip = node->find_port<InputPort>(*property);
      if (ip->is_connected()) {
        m_broken_connections.insert(std::make_unique<DisconnectPortsCommand>(*ip));
      }

      OutputPort* op = node->find_port<OutputPort>(*property);
      for (InputPort* ip : op->connected_inputs()) {
        m_broken_connections.insert(std::make_unique<DisconnectPortsCommand>(*ip));
      }
    }
  }
}

UserPropertyCommand::~UserPropertyCommand()
{
}

void UserPropertyCommand::undo()
{
  extract(m_additions);

  insert(m_deletions);
  for (auto&& disconnect_cmd : m_broken_connections) {
    disconnect_cmd->undo();
  }

  change(m_changes);
  Scene& scene = *m_owner.scene();
  Q_EMIT scene.message_box().selection_changed(scene.selection());
}

void UserPropertyCommand::redo()
{
  for (auto&& disconnect_cmd : m_broken_connections) {
    disconnect_cmd->redo();
  }
  extract(m_deletions);

  insert(m_additions);

  change(m_changes);
  Scene& scene = *m_owner.scene();
  Q_EMIT scene.message_box().selection_changed(scene.selection());
}

void UserPropertyCommand::extract(UserPropertyCommand::Properties& ps)
{
  for (auto&& [key, property] : ps) {
    assert(property == nullptr);
    property = m_owner.extract_property(key);
  }
}

void UserPropertyCommand::insert(UserPropertyCommand::Properties& ps)
{
  for (auto&& [key, property] : ps) {
    assert(property != nullptr);
    m_owner.add_property(key, std::move(property));
  }
}

void UserPropertyCommand::change(UserPropertyCommand::Configurations& configurations)
{
  for (auto&& [property, configuration] : configurations) {
    std::swap(property->configuration, configuration);
  }
}

}  // namespace omm



//    if (Node* node = kind_cast<Node*>(&m_owner); node != nullptr) {
//      PropertyPort<PortType::Input>* ip = node->find_port<InputPort>(*property);
//      assert(ip != nullptr);
//      if (ip->is_connected()) {
//        std::make_unique<DisconnectPortsCommand>(*ip);
//      }
//    }
