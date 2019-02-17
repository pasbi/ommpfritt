#pragma once

#include <string>
#include <functional>
#include <map>
#include <QKeySequence>

namespace omm
{

class CommandInterface
{
public:
  virtual void call(const std::string& command_name) = 0;
  virtual std::string type() const = 0;

protected:
  using Dispatcher = std::map<std::string, std::function<void(void)>>;
  bool dispatch(const std::string& command_name, const Dispatcher& dispatcher) const;
  ~CommandInterface() = default;
};

}  // namespace