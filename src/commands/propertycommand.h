#pragma once

#include "commands/command.h"
#include "properties/typedproperty.h"
#include "properties/numericproperty.h"
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
  std::set<Property*> m_properties;
};

template<typename PropertyT>
class PropertiesCommand : public AbstractPropertiesCommand
{
public:
  using value_type = typename PropertyT::value_type;
  class PropertyBiState
  {
  public:
    PropertyBiState(Property* property, const value_type& new_value)
      : property(property) , old_value(property->value<value_type>()), new_value(new_value) {}

    void undo() const { property->set(old_value); }
    void redo() const { property->set(new_value); }

    Property* property;
    const value_type old_value;
    value_type new_value;

    bool operator <(const PropertyBiState& other) const
    {
      return property < other.property;
    }
  };

protected:
  PropertiesCommand( const std::set<Property*>& properties,
                     const std::set<PropertyBiState>& properties_bi_states )
    : AbstractPropertiesCommand(properties), m_properties_bi_states(properties_bi_states) { }

private:
  template<typename Properties>
  static auto get_bi_states( const Properties& properties, const value_type& new_value)
  {
    return ::transform<PropertyBiState>(properties, [new_value](const auto& property) {
      return PropertyBiState(property, new_value);
    });
  }

public:
  PropertiesCommand(const std::set<Property*>& properties, const value_type& new_value)
    : PropertiesCommand(properties, get_bi_states(properties, new_value))
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
      // assert(::is_uniform(m_properties_bi_states, [](const auto pbs) { return pbs.new_value; }));
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

template<typename PropertyT, std::size_t dim>
class VectorPropertiesCommand : public PropertiesCommand<PropertyT>
{
  using value_type = typename PropertiesCommand<PropertyT>::value_type;
  using elem_type = typename value_type::elem_type;
  using PropertyBiState = typename PropertiesCommand<PropertyT>::PropertyBiState;

private:
  template<typename Properties>
  static auto get_bi_states(const Properties& properties, const elem_type& new_value)
  {
    return ::transform<PropertyBiState>(properties, [new_value](const auto& property) {
      auto new_vector_value = property->template value<value_type>();
      new_vector_value(dim) = new_value;
      return PropertyBiState(property, new_vector_value);
    });
  }

public:
  VectorPropertiesCommand(const std::set<Property*>& properties, const elem_type& new_value)
    : PropertiesCommand<PropertyT>(properties, get_bi_states(properties, new_value))
  {
  }
};

}  // namespace omm
