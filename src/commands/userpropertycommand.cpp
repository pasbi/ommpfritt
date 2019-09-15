#include "commands/userpropertycommand.h"
#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace omm
{

UserPropertyCommand
::UserPropertyCommand(const std::vector<std::string>& deletions,
                      std::vector<std::pair<std::string, std::unique_ptr<Property>>> additions,
                      const std::map<Property*, Property::Configuration>& changes,
                      AbstractPropertyOwner& owner)
  : Command(QObject::tr("User Property").toStdString())
  , m_deletions(deletions)
  , m_additions(std::move(additions))
  , m_changes(changes)
  , m_owner(owner)
{

}

void UserPropertyCommand::swap()
{
  // TODO order is not preserved
  std::vector<std::pair<std::string, std::unique_ptr<Property>>> removed;
  removed.reserve(m_deletions.size());
  for (const std::string& key : m_deletions) {
    removed.push_back(std::pair(key, m_owner.extract_property(key)));
  }

  m_deletions.clear();
  m_deletions.reserve(m_additions.size());
  for (auto&& [ key, property ] : m_additions) {
    m_owner.add_property(key, std::move(property));
    m_deletions.push_back(key);
  }

  m_additions = std::move(removed);

  for (auto& [ property, config ] : m_changes) {
    std::swap(property->configuration, config);
  }

  Scene& scene = *m_owner.scene();
  Q_EMIT scene.message_box().selection_changed(scene.selection());
}

}  // namespace omm
