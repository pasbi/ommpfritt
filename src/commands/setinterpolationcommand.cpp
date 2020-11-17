#include "commands/setinterpolationcommand.h"
#include "animation/track.h"
#include "properties/property.h"
#include <QObject>

namespace
{
auto collect(const std::set<omm::Property*>& properties, omm::Track::Interpolation interpolation)
{
  std::map<omm::Track*, omm::Track::Interpolation> map;
  for (omm::Property* p : properties) {
    if (p->track() != nullptr) {
      map.insert({p->track(), interpolation});
    }
  }
  return map;
}

}  // namespace

namespace omm
{
SetInterpolationCommand::SetInterpolationCommand(const std::set<Property*>& properties,
                                                 Track::Interpolation interpolation)
    : Command(QObject::tr("Set Interpolation")), m_others(collect(properties, interpolation))
{
}

void SetInterpolationCommand::swap()
{
  for (auto& [property, interpolation] : m_others) {
    const Track::Interpolation i = property->interpolation();
    property->set_interpolation(interpolation);
    interpolation = i;
  }
}

}  // namespace omm
