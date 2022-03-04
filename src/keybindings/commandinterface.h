#pragma once

#include "aspects/typed.h"
#include <QString>

namespace omm
{
class CommandInterface : virtual public Typed
{
public:
  ~CommandInterface() override = default;
  CommandInterface() = default;
  CommandInterface(CommandInterface&&) = delete;
  CommandInterface(const CommandInterface&) = delete;
  CommandInterface& operator=(CommandInterface&&) = delete;
  CommandInterface& operator=(const CommandInterface&) = delete;
  virtual bool perform_action(const QString& action_name) = 0;
};

}  // namespace omm
