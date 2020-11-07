#include "renderers/style.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/node.h"
#include "nodesystem/propertyport.h"
#include "nodesystem/nodemodel.h"
#include <QOpenGLFunctions>
#include "scene/mailbox.h"
#include "mainwindow/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "nodesystem/nodemodel.h"
#include "renderers/offscreenrenderer.h"
#include <QApplication>
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/styleiconengine.h"
#include "properties/optionproperty.h"
#include "objects/tip.h"
#include "scene/scene.h"
#include "properties/stringproperty.h"
#include <QOpenGLShaderProgram>

namespace {

static constexpr auto start_marker_prefix = "start";
static constexpr auto end_marker_prefix = "end";
static constexpr double default_marker_size = 2.0;
static constexpr auto default_marker_shape = omm::MarkerProperties::Shape::None;

}  // namespace

namespace omm
{

Style::Style(Scene *scene)
  : PropertyOwner(scene)
  , NodesOwner(AbstractNodeCompiler::Language::GLSL, *scene)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
  , m_offscreen_renderer(OffscreenRenderer::make())
{
  const auto pen_category = QObject::tr("pen");
  const auto brush_category = QObject::tr("brush");
  const auto decoration_category = QObject::tr("decoration");
  create_property<StringProperty>(NAME_PROPERTY_KEY, QObject::tr("<unnamed object>"))
    .set_label(QObject::tr("Name"))
    .set_category(QObject::tr("basic"));

  create_property<BoolProperty>(PEN_IS_ACTIVE_KEY, true)
    .set_label(QObject::tr("active"))
    .set_category(pen_category);
  create_property<ColorProperty>(PEN_COLOR_KEY, Colors::BLACK)
    .set_label(QObject::tr("color"))
    .set_category(pen_category);
  create_property<FloatProperty>(PEN_WIDTH_KEY, 5.0)
    .set_step(0.1)
    .set_range(0, std::numeric_limits<double>::infinity())
    .set_label(QObject::tr("width"))
    .set_category(pen_category);
  create_property<OptionProperty>(STROKE_STYLE_KEY, 0)
    .set_options({ QObject::tr("Solid"),
                   QObject::tr("Dashed"),
                   QObject::tr("Dotted"),
                   QObject::tr("DashDotted"),
                   QObject::tr("DashDotDotted") })
    .set_label(QObject::tr("Stroke Style")).set_category(pen_category);
  create_property<OptionProperty>(JOIN_STYLE_KEY, 2)
    .set_options({ QObject::tr("Bevel"),
                   QObject::tr("Miter"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Join")).set_category(pen_category);
  create_property<OptionProperty>(CAP_STYLE_KEY, 1)
    .set_options({ QObject::tr("Square"),
                   QObject::tr("Flat"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Cap")).set_category(pen_category);
  create_property<BoolProperty>(COSMETIC_KEY, true).set_label(QObject::tr("Cosmetic"))
      .set_category(pen_category);

  create_property<BoolProperty>(BRUSH_IS_ACTIVE_KEY, true)
    .set_label(QObject::tr("active"))
    .set_category(brush_category);
  create_property<ColorProperty>(BRUSH_COLOR_KEY, Colors::CERULEAN)
    .set_label(QObject::tr("color"))
    .set_category(brush_category);
  create_property<BoolProperty>("gl-brush", false)
    .set_label(tr("Use Nodes")).set_category(brush_category);

  create_property<TriggerProperty>(EDIT_NODES_PROPERTY_KEY)
    .set_label(QObject::tr("Edit ...")).set_category(brush_category);

  start_marker.make_properties(decoration_category);
  end_marker.make_properties(decoration_category);
  polish();
}

Style::~Style()
{
}

Style::Style(const Style &other)
  : PropertyOwner(other), NodesOwner(other)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
  , m_offscreen_renderer(std::make_unique<OffscreenRenderer>())
{
  other.copy_properties(*this, CopiedProperties::Compatible);
  polish();
}

void Style::polish()
{
  if (const NodeModel* model = node_model(); model != nullptr) {
    AbstractNodeCompiler& compiler = model->compiler();
    connect(&compiler, &AbstractNodeCompiler::compilation_succeeded, this, &Style::set_code);
    connect(&compiler, &AbstractNodeCompiler::compilation_failed, this, &Style::set_error);
    connect_edit_property(static_cast<TriggerProperty&>(*property(EDIT_NODES_PROPERTY_KEY)), *this);
  }
}

QString Style::type() const { return TYPE; }

std::unique_ptr<Style> Style::clone() const
{
  return std::make_unique<Style>(*this);
}

Flag Style::flags() const
{
  return Flag::None | Flag::HasGLSLNodes;
}

Texture Style::render_texture(const Object& object, const QSize& size, const QRectF& roi,
                              const Painter::Options& options) const
{
  update_uniform_values();
  return m_offscreen_renderer->render(object, size, roi, options);
}

void Style::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);
  if (NodeModel* node_model = this->node_model(); node_model != nullptr) {
    node_model->serialize(serializer, make_pointer(root, NODES_POINTER));
  }
}

void Style::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);
  if (NodeModel* node_model = this->node_model(); node_model != nullptr) {
    node_model->deserialize(deserializer, make_pointer(root, NODES_POINTER));
  }
}

void Style::on_property_value_changed(Property *property)
{
  if (    property == this->property(PEN_IS_ACTIVE_KEY)
       || property == this->property(PEN_COLOR_KEY)
       || property == this->property(PEN_WIDTH_KEY)
       || property == this->property(STROKE_STYLE_KEY)
       || property == this->property(JOIN_STYLE_KEY)
       || property == this->property(CAP_STYLE_KEY)
       || property == this->property(COSMETIC_KEY)
       || property == this->property(BRUSH_IS_ACTIVE_KEY)
       || property == this->property(BRUSH_COLOR_KEY) )
  {
    if (Scene* scene = this->scene(); scene != nullptr) {
      Q_EMIT scene->mail_box().appearance_changed(*this);
    }
  }
}

void Style::update_uniform_values() const
{
  if (const NodeModel* node_model = this->node_model(); node_model != nullptr) {
    auto& compiler = static_cast<NodeCompilerGLSL&>(node_model->compiler());
    for (AbstractPort* port : compiler.uniform_ports()) {
      assert(port->flavor == omm::PortFlavor::Property);
      const Property* property = port->port_type == PortType::Input
                                  ? static_cast<PropertyInputPort*>(port)->property()
                                  : static_cast<PropertyOutputPort*>(port)->property();
      if (property != nullptr) {
        m_offscreen_renderer->set_uniform(port->uuid(), property->variant_value());
      }
    }
  }
}

void Style::set_code(const QString& code) const
{
  if (NodeModel* node_model = this->node_model(); node_model != nullptr) {
    if (m_offscreen_renderer->set_fragment_shader(code)) {
      update_uniform_values();
      node_model->set_error("");
    } else {
      node_model->set_error(tr("Compilation failed"));
    }
  }
}

void Style::set_error(const QString& error) const
{
  if (NodeModel* node_model = this->node_model(); node_model != nullptr) {
    node_model->set_error(error);
    m_offscreen_renderer->set_fragment_shader("");
  }
}

}  // namespace omm
