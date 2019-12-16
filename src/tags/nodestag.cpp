#include "tags/nodestag.h"
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
  const auto code = property(ScriptTag::CODE_PROPERTY_KEY)->value<QString>();
  auto locals = py::dict( "this"_a=TagWrapper::make(*this),
                          "scene"_a=SceneWrapper(*scene) );
  scene->python_engine.exec(code, locals, this);
  owner->update();
}

void NodesTag::evaluate()
{
  if (property(UPDATE_MODE_PROPERTY_KEY)->value<std::size_t>() == 1) {
    force_evaluate();
  }
}

}  // namespace omm
