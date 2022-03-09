#include "tags/nodestag.h"
#include "external/pybind11/embed.h"
#include "main/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/spynode.h"
#include "nodesystem/port.h"
#include "nodesystem/propertyport.h"
#include "properties/triggerproperty.h"
#include "objects/object.h"

#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"

#include "common.h"
#include "python/pythonengine.h"
#include "python/scenewrapper.h"
#include "python/tagwrapper.h"

namespace py = pybind11;

namespace
{

template<omm::nodes::PortType port_type>
void populate_locals(py::object& locals, const omm::nodes::NodeModel& model)
{
  using PortT = omm::nodes::Port<port_type>;
  for (const auto* const port : model.ports<PortT>()) {
    if (port->flavor == omm::nodes::PortFlavor::Property) {
      using PropertyPort = omm::nodes::PropertyPort<port_type>;
      const auto* const property = static_cast<const PropertyPort*>(port)->property();
      if (property != nullptr) {
        const auto var_name = py::cast(port->uuid().toStdString());
        const auto var = variant_to_python(property->variant_value());
        locals[var_name] = var;
      }
    }
  }
}

using InputPropertyPort = omm::nodes::PropertyPort<omm::nodes::PortType::Input>;

void evaluate_connected_property_port(const InputPropertyPort& port, const pybind11::dict& locals)
{
  auto* const property = port.property();
  if (property != nullptr) {
    const auto var_name = port.uuid();
    const auto py_var_name = py::cast(var_name.toStdString());
    if (locals.contains(py_var_name)) {
      if (port.data_type() == property->data_type()) {
        const auto var = python_to_variant(locals[py_var_name], port.data_type());
        property->set(var);
      } else {
        // don't set the value if types don't match.
        // That may be inconvenient, but the type of the property is fixed. A value of another
        // type cannot be set.
      }
    }
  }
}

void evaluate_spy_node(omm::nodes::InputPort* port, const pybind11::dict& locals)
{
  auto& spy_node = dynamic_cast<omm::nodes::SpyNode&>(port->node);
  const auto py_var_name = py::cast(port->uuid().toStdString());
  if (locals.contains(py_var_name)) {
    py::object val = locals[py_var_name];
    const QString repr = QString::fromStdString(py::str(val));
    spy_node.set_text(repr);
  } else {
    spy_node.set_text(QObject::tr("nil"));
  }
}

}  // namespace

namespace omm
{
NodesTag::NodesTag(Object& owner)
    : Tag(owner), NodesOwner(nodes::BackendLanguage::Python, owner.scene())
{
  const QString category = QObject::tr("Basic");
  create_property<OptionProperty>(UPDATE_MODE_PROPERTY_KEY, 0)
      .set_options({QObject::tr("on request"), QObject::tr("per frame")})
      .set_label(QObject::tr("update"))
      .set_category(category);
  create_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
      .set_label(QObject::tr("evaluate"))
      .set_category(category);
  create_property<TriggerProperty>(EDIT_NODES_PROPERTY_KEY)
      .set_label(QObject::tr("Edit ..."))
      .set_category(category);
  polish();
}

NodesTag::NodesTag(const NodesTag& other) : Tag(other), NodesOwner(other)
{
  polish();
}

NodesTag::~NodesTag() = default;

QString NodesTag::type() const
{
  return TYPE;
}
Flag NodesTag::flags() const
{
  return Tag::flags() | Flag::HasPythonNodes;
}

std::set<nodes::Node*> NodesTag::nodes() const
{
  return node_model().nodes();
}

void NodesTag::serialize(serialization::SerializerWorker& worker) const
{
  Tag::serialize(worker);
  node_model().serialize(*worker.sub(NODES_POINTER));
}

void NodesTag::deserialize(serialization::DeserializerWorker& worker)
{
  Tag::deserialize(worker);
  node_model().deserialize(*worker.sub(NODES_POINTER));
}

void NodesTag::polish()
{
  connect_edit_property(dynamic_cast<TriggerProperty&>(*property(EDIT_NODES_PROPERTY_KEY)), *this);
}

void NodesTag::on_property_value_changed(Property* property)
{
  if (property == this->property(TRIGGER_UPDATE_PROPERTY_KEY)) {
    force_evaluate();
  }
}

void NodesTag::force_evaluate()
{
  using namespace py::literals;

  auto locals = py::dict();
  auto& model = node_model();
  populate_locals<nodes::PortType::Input>(locals, model);
  populate_locals<nodes::PortType::Output>(locals, model);

  const auto code = model.compiler().code();
  if (PythonEngine::instance().exec(code, locals, this)) {
    for (auto* const port : model.ports<nodes::InputPort>()) {
      if (port->node.type() == nodes::SpyNode::TYPE) {
        ::evaluate_spy_node(port, locals);
      }
      if (port->flavor == nodes::PortFlavor::Property && port->is_connected()) {
        ::evaluate_connected_property_port(dynamic_cast<const InputPropertyPort&>(*port), locals);
      }
    }
    model.set_error("");
  } else {
    model.set_error(tr("Fail."));
  }
  owner->update();
}

void NodesTag::evaluate()
{
  if (property(UPDATE_MODE_PROPERTY_KEY)->value<std::size_t>() == 1) {
    force_evaluate();
  }
}

}  // namespace omm
