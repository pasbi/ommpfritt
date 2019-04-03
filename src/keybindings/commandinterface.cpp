#include "keybindings/commandinterface.h"
#include "logging.h"

namespace omm
{

bool CommandInterface::dispatch(const std::string& command, const Dispatcher& dispatcher) const
{
  const auto it = dispatcher.find(command);
  if (it != dispatcher.end()) {
    it->second();
    return true;
  } else {
    LWARNING << "Failed to dispatch command '" << command << "' in '" << type() << "'.";
    return false;
  }
}

}  // namespace omm
