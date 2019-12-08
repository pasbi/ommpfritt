#include "renderers/style.h"
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
#include "scene/messagebox.h"

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
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
  , m_nodes(std::make_unique<NodeModel>(scene))
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
  create_property<TriggerProperty>(EDIT_NODES_KEY)
    .set_label(tr("Edit Nodes ...")).set_category(brush_category);
  create_property<BoolProperty>("gl-brush", false)
    .set_label(tr("Use Nodes")).set_category(brush_category);

  start_marker.make_properties(decoration_category);
  end_marker.make_properties(decoration_category);
  init_offscreen_renderer();
}

Style::~Style()
{
}

Style::Style(const Style &other)
  : PropertyOwner(other)
  , start_marker(start_marker_prefix, *this, default_marker_shape, default_marker_size)
  , end_marker(end_marker_prefix, *this, default_marker_shape, default_marker_size)
{
  init_offscreen_renderer();
}

QString Style::type() const { return TYPE; }
AbstractPropertyOwner::Flag Style::flags() const { return Flag::None; }

QPixmap Style::texture(const Object& object, const QSize& size) const
{
  Q_UNUSED(object)
  return QPixmap::fromImage(m_offscreen_renderer->render(size));
}

void Style::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  PropertyOwner::serialize(serializer, root);
  m_nodes->serialize(serializer, make_pointer(root, NODES_POINTER));
}

void Style::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  PropertyOwner::deserialize(deserializer, root);
  m_nodes->deserialize(deserializer, make_pointer(root, NODES_POINTER));
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
  } else if (property == this->property(EDIT_NODES_KEY)) {
    for (NodeManager* nm : Application::instance().managers<NodeManager>()) {
      nm->set_model(m_nodes.get());
    }
  }
}

void Style::init_offscreen_renderer()
{
  static constexpr auto fragment_code = R"(
varying highp vec3 vert;
#define M_PI 3.1415926535897932384626433832795

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
   float arg = atan(vert.y, vert.x);
   float r = sqrt(vert.y * vert.y + vert.x * vert.x);
   vec3 hsv = vec3(arg / (2.0 * M_PI), r, 1.0);
   gl_FragColor = vec4(hsv2rgb(hsv), 1.0);
}

)";
  m_offscreen_renderer = std::make_unique<OffscreenRenderer>();
  m_offscreen_renderer->set_fragment_shader(fragment_code);
}

std::unique_ptr<Style> Style::clone() const
{
  auto clone = std::make_unique<Style>(scene());
  copy_properties(*clone);
  return clone;
}

}  // namespace omm
