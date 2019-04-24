#pragma once

#include <string>
#include <functional>
#include <map>
#include <QKeySequence>
#include "keybindings/keybinding.h"
#include "logging.h"

namespace omm
{

class CommandInterface
{
public:
  virtual std::string type() const = 0;

  template<typename CommandInterfaceT> struct ActionInfo
  {
    using ActionCallback = std::function<void(CommandInterfaceT&)>;
    ActionInfo(const std::string& name, const QKeySequence& key_sequence, const ActionCallback& f)
      : name(name), default_key_sequence(key_sequence), callback(f) {}

    std::string name;
    QKeySequence default_key_sequence;
    ActionCallback callback;
    KeyBinding make_keybinding() const
    {
      return KeyBinding(name, CommandInterfaceT::TYPE, default_key_sequence);
    }
  };

  template<typename CommandInterfaceT> void call(const std::string& command_name)
  {
    assert(this->type() == CommandInterfaceT::TYPE);
    const auto infos = CommandInterfaceT::action_infos();
    const auto it = std::find_if(infos.begin(), infos.end(), [command_name](const auto& info) {
      return info.name == command_name;
    });
    if (it == infos.end()) {
      LERROR << "Failed to dispatch command '" << CommandInterfaceT::TYPE
                 << "'::'" << command_name << "'.";
    } else {
      it->callback(static_cast<CommandInterfaceT&>(*this));
    }
  }

  // any derivating class T must provide
  // static std::vector<ActionInfo<T>> action_infos();

protected:
  using Dispatcher = std::map<std::string, std::function<void(void)>>;
  bool dispatch(const std::string& command_name, const Dispatcher& dispatcher) const;
  ~CommandInterface() = default;
};

}  // namespace
