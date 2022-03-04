#include "renderers/style.h"
#include "main/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/propertyport.h"
#include "nodesystem/nodecompilerglsl.h"
#include "objects/tip.h"
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"
#include "properties/triggerproperty.h"
#include "properties/propertygroups/markerproperties.h"
#include "renderers/offscreenrenderer.h"
#include "renderers/styleiconengine.h"
#include "renderers/texture.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <QApplication>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

namespace
{

constexpr auto start_marker_prefix = "start";
constexpr auto end_marker_prefix = "end";

auto make_default_marker_properties(const QString& prefix, omm::Style& style)
{
  constexpr double default_marker_size = 2.0;
  constexpr auto default_marker_shape = omm::MarkerProperties::Shape::None;
  return std::make_unique<omm::MarkerProperties>(prefix,
                                                 style,
                                                 default_marker_shape,
                                                 default_marker_size);
}

}  // namespace

namespace omm
{
Style::Style(Scene* scene)
    : PropertyOwner(scene), NodesOwner(nodes::BackendLanguage::GLSL, scene)
    , start_marker(make_default_marker_properties(start_marker_prefix,*this))
    , end_marker(make_default_marker_properties(end_marker_prefix, *this))
    , m_offscreen_renderer(OffscreenRenderer::make())
{
  static constexpr double DEFAULT_PEN_WIDTH = 5.0;
  static constexpr double PEN_WIDTH_STEP = 0.1;
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
  create_property<FloatProperty>(PEN_WIDTH_KEY, DEFAULT_PEN_WIDTH)
      .set_step(PEN_WIDTH_STEP)
      .set_range(0, std::numeric_limits<double>::infinity())
      .set_label(QObject::tr("width"))
      .set_category(pen_category);
  create_property<OptionProperty>(STROKE_STYLE_KEY, 0)
      .set_options({QObject::tr("Solid"),
                    QObject::tr("Dashed"),
                    QObject::tr("Dotted"),
                    QObject::tr("DashDotted"),
                    QObject::tr("DashDotDotted")})
      .set_label(QObject::tr("Stroke Style"))
      .set_category(pen_category);
  create_property<OptionProperty>(JOIN_STYLE_KEY, 2)
      .set_options({QObject::tr("Bevel"), QObject::tr("Miter"), QObject::tr("Round")})
      .set_label(QObject::tr("Join"))
      .set_category(pen_category);
  create_property<OptionProperty>(CAP_STYLE_KEY, 1)
      .set_options({QObject::tr("Square"), QObject::tr("Flat"), QObject::tr("Round")})
      .set_label(QObject::tr("Cap"))
      .set_category(pen_category);
  create_property<BoolProperty>(COSMETIC_KEY, true)
      .set_label(QObject::tr("Cosmetic"))
      .set_category(pen_category);

  create_property<BoolProperty>(BRUSH_IS_ACTIVE_KEY, true)
      .set_label(QObject::tr("active"))
      .set_category(brush_category);
  create_property<ColorProperty>(BRUSH_COLOR_KEY, Colors::CERULEAN)
      .set_label(QObject::tr("color"))
      .set_category(brush_category);
  create_property<BoolProperty>("gl-brush", false)
      .set_label(tr("Use Nodes"))
      .set_category(brush_category);

  create_property<TriggerProperty>(EDIT_NODES_PROPERTY_KEY)
      .set_label(QObject::tr("Edit ..."))
      .set_category(brush_category);

  start_marker->make_properties(decoration_category);
  end_marker->make_properties(decoration_category);
  polish();
}

Style::~Style() = default;

Style::Style(const Style& other)
    : PropertyOwner(other), NodesOwner(other)
    , start_marker(make_default_marker_properties(start_marker_prefix, *this))
    , end_marker(make_default_marker_properties(end_marker_prefix, *this))
    , m_offscreen_renderer(std::make_unique<OffscreenRenderer>())
{
  other.copy_properties(*this, CopiedProperties::Compatible);
  polish();
}

void Style::polish()
{
  if (const auto& model = node_model(); model.is_enabled()) {
    auto& compiler = model.compiler();
    connect(&compiler, &nodes::AbstractNodeCompiler::compilation_succeeded, this, &Style::set_code);
    connect(&compiler, &nodes::AbstractNodeCompiler::compilation_failed, this, &Style::set_error);
    connect_edit_property(dynamic_cast<TriggerProperty&>(*property(EDIT_NODES_PROPERTY_KEY)),
                          *this);
  }
}

QString Style::type() const
{
  return TYPE;
}

std::unique_ptr<Style> Style::clone() const
{
  return std::make_unique<Style>(*this);
}

Flag Style::flags() const
{
  return Flag::None | Flag::HasGLSLNodes;
}

Texture Style::render_texture(const Object& object,
                              const QSize& size,
                              const QRectF& roi,
                              const PainterOptions& options) const
{
  update_uniform_values();
  return m_offscreen_renderer->render(object, size, roi, options);
}

void Style::serialize(serialization::SerializerWorker& worker) const
{
  PropertyOwner::serialize(worker);
  node_model().serialize(*worker.sub(NODES_POINTER));
}

void Style::deserialize(serialization::DeserializerWorker& worker)
{
  PropertyOwner::deserialize(worker);
  node_model().deserialize(*worker.sub(NODES_POINTER));
}

void Style::on_property_value_changed(Property* property)
{
  if (property == this->property(PEN_IS_ACTIVE_KEY) || property == this->property(PEN_COLOR_KEY)
      || property == this->property(PEN_WIDTH_KEY) || property == this->property(STROKE_STYLE_KEY)
      || property == this->property(JOIN_STYLE_KEY) || property == this->property(CAP_STYLE_KEY)
      || property == this->property(COSMETIC_KEY) || property == this->property(BRUSH_IS_ACTIVE_KEY)
      || property == this->property(BRUSH_COLOR_KEY)) {
    if (Scene* scene = this->scene(); scene != nullptr) {
      Q_EMIT scene->mail_box().style_appearance_changed(*this);
    }
  }
}

void Style::update_uniform_values() const
{
  if (const auto& node_model = this->node_model(); node_model.is_enabled()) {
    auto& compiler = dynamic_cast<nodes::NodeCompilerGLSL&>(node_model.compiler());
    for (auto* const port : compiler.uniform_ports()) {
      assert(port->flavor == nodes::PortFlavor::Property);
      const Property* property = port->port_type == nodes::PortType::Input
                                     ? dynamic_cast<nodes::PropertyInputPort*>(port)->property()
                                     : dynamic_cast<nodes::PropertyOutputPort*>(port)->property();
      if (property != nullptr) {
        m_offscreen_renderer->set_uniform(port->uuid(), property->variant_value());
      }
    }
  }
}

void Style::set_code(const QString& code) const
{
  if (auto& node_model = this->node_model(); node_model.is_enabled()) {
    if (m_offscreen_renderer->set_fragment_shader(code)) {
      update_uniform_values();
      node_model.set_error("");
    } else {
      node_model.set_error(tr("Compilation failed"));
    }
  }
}

void Style::set_error(const QString& error) const
{
  if (auto& node_model = this->node_model(); node_model.is_enabled()) {
    node_model.set_error(error);
    m_offscreen_renderer->set_fragment_shader("");
  }
}

}  // namespace omm
