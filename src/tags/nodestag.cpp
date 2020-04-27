#include "tags/nodestag.h"
#include "nodesystem/nodes/spynode.h"
#include "mainwindow/application.h"
#include "nodesystem/propertyport.h"
#include "nodesystem/port.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/nodemodel.h"
#include "managers/nodemanager/nodemanager.h"
#include "mainwindow/application.h"
#include "properties/triggerproperty.h"
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "properties/triggerproperty.h"

#include "python/tagwrapper.h"
#include "python/scenewrapper.h"
#include "python/pythonengine.h"
#include "common.h"

namespace py = pybind11;

namespace
{

template<omm::PortType port_type>
void populate_locals(py::object& locals, const omm::NodeModel& model)
{
  using PortT = omm::Port<port_type>;
  for (PortT* port : model.ports<PortT>()) {
    if (port->flavor == omm::PortFlavor::Property) {
      omm::Property* property = static_cast<omm::PropertyPort<port_type>*>(port)->property();
      if (property != nullptr) {
        const auto var_name = py::cast(port->uuid().toStdString());
        const auto var = variant_to_python(property->variant_value());
        locals[var_name] = var;
      }
    }
  }
}

}  // namespace

namespace omm
{

NodesTag::NodesTag(Object& owner)
  : Tag(owner), NodesOwner(AbstractNodeCompiler::Language::Python, *owner.scene())
{
  const QString category = QObject::tr("Basic");
  create_property<OptionProperty>(UPDATE_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("on request"), QObject::tr("per frame") })
    .set_label(QObject::tr("update")).set_category(category);
  create_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
    .set_label(QObject::tr("evaluate")).set_category(category);
  add_property(EDIT_NODES_PROPERTY_KEY, make_edit_nodes_property())
    .set_label(QObject::tr("Edit ...")).set_category(category);
}

NodesTag::NodesTag(const NodesTag& other)
  : Tag(other), NodesOwner(other)
{
}

NodesTag::~NodesTag()
{
}

QString NodesTag::type() const { return TYPE; }
Flag NodesTag::flags() const
{
  return Tag::flags() | Flag::HasPythonNodes;
}

std::set<Node*> NodesTag::nodes() const
{
  return node_model()->nodes();
}

void NodesTag::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  Tag::serialize(serializer, root);
  node_model()->serialize(serializer, make_pointer(root, NODES_POINTER));
}

void NodesTag::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  Tag::deserialize(deserializer, root);
  node_model()->deserialize(deserializer, make_pointer(root, NODES_POINTER));
}


void NodesTag::on_property_value_changed(Property *property)
{
  if (property == this->property(TRIGGER_UPDATE_PROPERTY_KEY)) {
    force_evaluate();
  }
}

void NodesTag::force_evaluate()
{
  Scene* scene = owner->scene();
  assert(scene != nullptr);
  using namespace py::literals;

  auto locals = py::dict();
  NodeModel& model = *node_model();
  populate_locals<PortType::Input>(locals, model);
  populate_locals<PortType::Output>(locals, model);

  const auto code = model.compiler().code();
  if (Application::instance().python_engine.exec(code, locals, this)) {
    for (InputPort* port : model.ports<InputPort>()) {
      if (port->node.type() == SpyNode::TYPE) {
        SpyNode& spy_node = static_cast<SpyNode&>(port->node);
        const auto py_var_name = py::cast(port->uuid().toStdString());
        if (locals.contains(py_var_name)) {
          py::object val = locals[py_var_name];
          const QString repr = QString::fromStdString(py::str(val));
          spy_node.set_text(repr);
        } else {
          spy_node.set_text(tr("nil"));
        }
      }
      if (port->flavor == PortFlavor::Property && port->is_connected()) {
        Property* property = static_cast<PropertyPort<PortType::Input>*>(port)->property();
        if (property != nullptr) {
          const auto var_name = port->uuid();
          const auto py_var_name =  py::cast(var_name.toStdString());
          if (locals.contains(py_var_name)) {
            if (port->data_type() == property->data_type()) {
              const variant_type var = python_to_variant(locals[py_var_name], port->data_type());
              property->set(var);
            } else {
              // don't set the value if types don't match.
              // That may be inconvenient, but the type of the property is fixed. A value of another
              // type cannot be set.
            }
          }
        }
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
