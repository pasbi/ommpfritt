#include "tags/nodestag.h"
#include "managers/nodemanager/propertyport.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/nodecompiler.h"
#include "managers/nodemanager/nodemodel.h"
#include "managers/nodemanager/nodemanager.h"
#include "mainwindow/application.h"
#include "properties/triggerproperty.h"
#include <pybind11/embed.h>

#include "properties/stringproperty.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "properties/triggerproperty.h"

#include "python/tagwrapper.h"
#include "python/scenewrapper.h"
#include "python/pythonengine.h"
#include "common.h"

namespace py = pybind11;

namespace omm
{

NodesTag::NodesTag(Object& owner)
  : Tag(owner)
  , m_nodes(std::make_unique<NodeModel>(owner.scene()))
  , m_compiler_cache(*this)
{
  const QString category = QObject::tr("Nodes");
  create_property<OptionsProperty>(UPDATE_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("on request"), QObject::tr("per frame") })
    .set_label(QObject::tr("update")).set_category(category);
  create_property<TriggerProperty>(TRIGGER_UPDATE_PROPERTY_KEY)
    .set_label(QObject::tr("evaluate")).set_category(category);
  create_property<TriggerProperty>(EDIT_NODES_KEY)
      .set_label(tr("Edit Nodes ...")).set_category(category);
}

NodesTag::NodesTag(const NodesTag& other)
  : Tag(other)
  , m_nodes(std::make_unique<NodeModel>(*other.m_nodes))
  , m_compiler_cache(*this)
{
}

NodesTag::~NodesTag()
{
}

QString NodesTag::type() const { return TYPE; }
AbstractPropertyOwner::Flag NodesTag::flags() const { return Tag::flags(); }

void NodesTag::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  Tag::serialize(serializer, root);
  m_nodes->serialize(serializer, make_pointer(root, NODES_POINTER));
}

void NodesTag::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  Tag::deserialize(deserializer, root);
  m_nodes->deserialize(deserializer, make_pointer(root, NODES_POINTER));
}

std::unique_ptr<Tag> NodesTag::clone() const { return std::make_unique<NodesTag>(*this); }

void NodesTag::on_property_value_changed(Property *property)
{
  if (property == this->property(TRIGGER_UPDATE_PROPERTY_KEY)) {
    force_evaluate();
  } else if (property == this->property(EDIT_NODES_KEY)) {
    for (NodeManager* nm : Application::instance().managers<NodeManager>()) {
      nm->set_model(m_nodes.get());
    }
  }
}

void NodesTag::force_evaluate()
{
  Scene* scene = owner->scene();
  assert(scene != nullptr);
  using namespace py::literals;

  const auto code = m_compiler_cache();
  LINFO << "Compilation: \n" << code;
  auto locals = py::dict();
  const NodeCompiler* compiler = m_compiler_cache.compiler();
  for (OutputPort* port : m_nodes->ports<OutputPort>()) {
    if (port->flavor == PortFlavor::Property) {
      Property& property = static_cast<PropertyPort<PortType::Output>*>(port)->property;
      const auto var_name = py::cast(compiler->uuid(*port).toStdString());
      locals[var_name] = variant_to_python(property.variant_value());
    }
  }
  scene->python_engine.exec(code, locals, this);
  for (InputPort* port : m_nodes->ports<InputPort>()) {
    if (port->flavor == PortFlavor::Property) {
      Property& property = static_cast<PropertyPort<PortType::Input>*>(port)->property;
      const auto var_name = py::cast(compiler->uuid(*port).toStdString());
      if (locals.contains(var_name)) {
        const variant_type var = python_to_variant(locals[var_name], property.data_type());
        property.set(var);
      }
    }
  }
  owner->update();
}

void NodesTag::evaluate()
{
  if (property(UPDATE_MODE_PROPERTY_KEY)->value<std::size_t>() == 1) {
    force_evaluate();
  }
}

NodesTag::CompilerCache::CompilerCache(NodesTag& self) : CachedGetter<QString, NodesTag>(self)
{
  connect(self.m_nodes.get(), &NodeModel::topology_changed, [this]() {
    invalidate();
  });
}

NodesTag::CompilerCache::~CompilerCache()
{
  
}

QString NodesTag::CompilerCache::compute() const
{
  m_compiler = std::make_unique<NodeCompiler>(NodeCompiler::Language::Python);
  m_compiler->compile(*m_self.m_nodes);
  return m_compiler->compilation();
}

}  // namespace omm
