#pragma once

#include <string>

namespace omm
{

class CommandInterface
{
public:
  virtual void call(const std::string& command_name) = 0;
  virtual std::string type() const = 0;

protected:
  ~CommandInterface() = default;
};

}  // namespace