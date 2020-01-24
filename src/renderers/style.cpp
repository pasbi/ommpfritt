#include "renderers/style.h"
#include "managers/nodemanager/nodes/fragmentnode.h"
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/propertyport.h"
#include "managers/nodemanager/nodemodel.h"
#include <QOpenGLFunctions>
#include "scene/messagebox.h"
#include "mainwindow/application.h"
#include "managers/nodemanager/nodemanager.h"
#include "managers/nodemanager/nodemodel.h"
#include "renderers/offscreenrenderer.h"
#include <QApplication>
#include "properties/boolproperty.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/triggerproperty.h"
#include "renderers/styleiconengine.h"
#include "properties/optionsproperty.h"
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
  : PropertyOwner(scene), NodesOwner(*scene)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
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
  create_property<FloatProperty>(PEN_WIDTH_KEY, 1.0)
    .set_step(0.1)
    .set_range(0, std::numeric_limits<double>::infinity())
    .set_label(QObject::tr("width"))
    .set_category(pen_category);
  create_property<OptionsProperty>(STROKE_STYLE_KEY, 0)
    .set_options({ QObject::tr("Solid"),
                   QObject::tr("Dashed"),
                   QObject::tr("Dotted"),
                   QObject::tr("DashDotted"),
                   QObject::tr("DashDotDotted") })
    .set_label(QObject::tr("Stroke Style")).set_category(pen_category);
  create_property<OptionsProperty>(JOIN_STYLE_KEY, 2)
    .set_options({ QObject::tr("Bevel"),
                   QObject::tr("Miter"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Join")).set_category(pen_category);
  create_property<OptionsProperty>(CAP_STYLE_KEY, 1)
    .set_options({ QObject::tr("Square"),
                   QObject::tr("Flat"),
                   QObject::tr("Round") })
    .set_label(QObject::tr("Cap")).set_category(pen_category);
  create_property<BoolProperty>(COSMETIC_KEY, true).set_label(QObject::tr("Cosmetic"))
      .set_category(pen_category);

  create_property<BoolProperty>(BRUSH_IS_ACTIVE_KEY, false)
    .set_label(QObject::tr("active"))
    .set_category(brush_category);
  create_property<ColorProperty>(BRUSH_COLOR_KEY, Colors::RED)
    .set_label(QObject::tr("color"))
    .set_category(brush_category);
  create_property<BoolProperty>("gl-brush", false)
    .set_label(tr("Use Nodes")).set_category(brush_category);

  add_property(EDIT_NODES_PROPERTY_KEY, make_edit_nodes_property())
    .set_label(QObject::tr("Edit ...")).set_category(brush_category);

  start_marker.make_properties(decoration_category);
  end_marker.make_properties(decoration_category);
}

Style::~Style()
{
}

Style::Style(const Style &other)
  : PropertyOwner(other), NodesOwner(other)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
{
}

QString Style::type() const { return TYPE; }
Flag Style::flags() const
{
  return Flag::None | Flag::HasGLSLNodes;
}

QPixmap Style::texture(const Object& object, const QSize& size) const
{
  Q_UNUSED(object)
  if (!m_offscreen_renderer) {
    m_offscreen_renderer = init_offscreen_renderer();
    update_uniform_values();
  }
  return QPixmap::fromImage(m_offscreen_renderer->render(size));
}

void Style::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);
  node_model().serialize(serializer, make_pointer(root, NODES_POINTER));
}

void Style::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);
  node_model().deserialize(deserializer, make_pointer(root, NODES_POINTER));
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
      Q_EMIT scene->message_box().appearance_changed(*this);
    }
  }
}

std::unique_ptr<OffscreenRenderer> Style::init_offscreen_renderer() const
{
  auto offscreen_renderer = std::make_unique<OffscreenRenderer>();
  connect(compiler(), &AbstractNodeCompiler::compilation_succeeded,
          [this, o_r = offscreen_renderer.get()](const QString& code)
  {
    o_r->set_fragment_shader(code);
    update_uniform_values();
  });
  connect(&scene()->message_box(), &MessageBox::property_value_changed,
          [this](AbstractPropertyOwner&, const QString&, Property&)
  {
    update_uniform_values();
  });
  offscreen_renderer->set_fragment_shader(code());
  return offscreen_renderer;
}

void Style::update_uniform_values() const
{
  if (m_offscreen_renderer) {
    for (AbstractPort* port : compiler()->uniform_ports()) {
      assert(port->flavor == omm::PortFlavor::Property);
      const Property* property = static_cast<const PropertyInputPort*>(port)->property();
      m_offscreen_renderer->set_uniform(port->uuid(), property->variant_value());
    }
  }
}

std::unique_ptr<Style> Style::clone() const
{
  auto clone = std::make_unique<Style>(scene());
  copy_properties(*clone);
  return clone;
}

}  // namespace omm
