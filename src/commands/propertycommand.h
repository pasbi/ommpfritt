#pragma once

#include "commands/command.h"
#include "properties/typedproperty.h"
#include "common.h"

namespace omm
{

class AbstractPropertiesCommand : public Command
{
public:
  AbstractPropertiesCommand(const std::set<Property*>& properties);
  bool mergeWith(const QUndoCommand* command) override = 0;
  int id() const override;

private:
  Property::SetOfProperties m_properties;
};

template<typename PropertyT>
class PropertiesCommand : public AbstractPropertiesCommand
{
  using value_type = typename PropertyT::value_type;
  class PropertyBiState
  {
  public:
    PropertyBiState(PropertyT* property, const value_type& new_value)
      : property(property) , old_value(property->value()), new_value(new_value) {}

    void undo() const { property->set_value(old_value); }
    void redo() const { property->set_value(new_value); }

    PropertyT* property;
    const value_type old_value;
    value_type new_value;

    bool operator <(const PropertyBiState& other) const
    {
      return property < other.property;
    }
  };

  template<typename Properties>
  static auto get_bi_states( const Properties& properties, const value_type& new_value)
  {
    return ::transform<PropertyBiState>(properties, [new_value](const auto& property) {
      return PropertyBiState(property, new_value);
    });
  }

public:
  PropertiesCommand(const std::set<PropertyT*>& properties, const value_type& new_value)
    : AbstractPropertiesCommand(Property::cast_all<PropertyT>(properties))
    , m_properties_bi_states(get_bi_states(properties, new_value))
  {
  }

  void undo() override
  {
    for (const auto& property_bi_state : m_properties_bi_states) { property_bi_state.undo(); }
  }

  void redo() override
  {
    for (const auto& property_bi_state : m_properties_bi_states) { property_bi_state.redo(); }
  }

  bool mergeWith(const QUndoCommand* command) override
  {
    if (AbstractPropertiesCommand::mergeWith(command)) {
      const auto& property_command = static_cast<const PropertiesCommand<PropertyT>&>(*command);
      assert(::is_uniform(m_properties_bi_states, [](const auto pbs) { return pbs.new_value; }));
      const value_type new_value = m_properties_bi_states.begin()->new_value;
      for (auto pbs : m_properties_bi_states) {
        pbs.new_value = new_value;
      }
      return true;
    } else {
      return false;
    }
  }

private:
  std::set<PropertyBiState> m_properties_bi_states;
};

}  // namespace omm