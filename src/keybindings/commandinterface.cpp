#include "keybindings/commandinterface.h"
#include <glog/logging.h>

namespace omm
{

bool CommandInterface::dispatch(const std::string& command, const Dispatcher& dispatcher) const
{
  const auto it = dispatcher.find(command);
  if (it != dispatcher.end()) {
    LOG(INFO) << "dispatch command " << command;
    it->second();
    return true;
  } else {
    LOG(WARNING) << "Failed to dispatch command '" << command << "' in '" << type() << "'.";
    return false;
  }
}

}  // namespace omm
