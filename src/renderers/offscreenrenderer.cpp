#include "offscreenrenderer.h"
#include "geometry/objecttransformation.h"
#include "main/application.h"
#include "main/options.h"
#include "nodesystem/nodecompilerglsl.h"
#include "objects/object.h"
#include "renderers/texture.h"
#include "renderers/painteroptions.h"
#include <QApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace omm
{
const std::vector<OffscreenRenderer::ShaderInput> OffscreenRenderer::fragment_shader_inputs = {
    {
        Type::FloatVector,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "local_pos"),
        ShaderInput::Kind::Varying,
    },
    {
        Type::FloatVector,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "global_pos"),
        ShaderInput::Kind::Varying,
    },
    {
        Type::FloatVector,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "local_normalized_pos"),
        ShaderInput::Kind::Varying,
    },
    {
        Type::FloatVector,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "object_size"),
        ShaderInput::Kind::Uniform,
    },
    {
        Type::FloatVector,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "view_pos"),
        ShaderInput::Kind::Varying,
    },
    {
        Type::Integer,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "object_id"),
        ShaderInput::Kind::Uniform,
    },
    {
        Type::Integer,
        QT_TRANSLATE_NOOP("OffscreenRenderer", "path_id"),
        ShaderInput::Kind::Uniform,
    },
};

QString OffscreenRenderer::ShaderInput::tr_name() const
{
  return QApplication::translate("OffscreenRenderer", name);
}

}  // namespace omm

namespace
{
constexpr auto vertex_position_attribute_name = "vertex_attr";

using S = omm::OffscreenRenderer;
constexpr auto vertex_code = R"(
#version 140

attribute vec4 vertex_attr;
varying vec2 local_pos;
varying vec2 local_normalized_pos;
varying vec2 global_pos;
varying vec2 view_pos;
uniform vec2 object_size;
uniform mat3 global_transform;
uniform mat3 view_transform;
uniform vec2 view_size;
uniform vec2 roi_tl;
uniform vec2 roi_br;
uniform int object_id;
uniform int path_id;

vec2 unc(vec2 centered) {
  return (centered + vec2(1.0, 1.0)) / 2.0;
}

vec2 c(vec2 uncentered) {
  return (uncentered * 2.0) - vec2(1.0, 1.0);
}

void main() {
  vec2 lncp = vertex_attr.xy * vec2(1.0, -1.0);

  lncp = unc(lncp);
  lncp *= (roi_br - roi_tl) / 2.0;
  lncp += (roi_tl + vec2(1.0, 1.0))/2.0;
  lncp = c(lncp);

  vec2 local_normalized_centered_pos = lncp;
  vec2 local_centered_pos = local_normalized_centered_pos * object_size / 2.0;
  global_pos = (global_transform * vec3(local_centered_pos, 1.0)).xy;
  view_pos = (view_transform * vec3(local_centered_pos, 1.0)).xy / view_size;
  local_pos = unc(local_normalized_centered_pos) * object_size / 2.0;
  local_normalized_pos = unc(local_normalized_centered_pos);
  gl_Position = vec4(vertex_attr.xy, 0.0, 1.0);
}
)";

constexpr std::array<float, 18> m_quad = {
    -1.0,
    -1.0,
    0.0,
    1.0,
    1.0,
    0.0,
    -1.0,
    1.0,
    0.0,

    -1.0,
    -1.0,
    0.0,
    1.0,
    -1.0,
    0.0,
    1.0,
    1.0,
    0.0,
};

template<typename T> std::vector<T> sample(const omm::SplineType& spline, std::size_t n)
{
  std::vector<T> values;
  values.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    const double t = static_cast<double>(i) / static_cast<double>(n - 1);
    const double v = spline.evaluate(t).value();
    values.push_back(v);
  }
  return values;
}

template<typename T>
void set_uniform(omm::OffscreenRenderer& self, const QString& name, const T& value)
{
  using namespace omm;
  self.make_current();
  auto* program = self.program();
  if (program == nullptr) {
    // the compilation has failed for whatever reason
    return;
  }
  program->bind();
  [[maybe_unused]] GLuint location = program->uniformLocation(name);
  if constexpr (std::is_same_v<T, double>) {
    program->setUniformValue(location, GLfloat(value));
  } else if constexpr (std::is_same_v<T, int>) {
    program->setUniformValue(location, value);
  } else if constexpr (std::is_same_v<T, AbstractPropertyOwner*>) {
    if (value == nullptr) {
      program->setUniformValue(location, 0);
    } else {
      program->setUniformValue(location, GLuint(value->id()));
    }
  } else if constexpr (std::is_same_v<T, Color>) {
    auto [r, g, b, a] = value.components(Color::Model::RGBA);
    program->setUniformValue(location, QVector4D(r, g, b, a));
  } else if constexpr (std::is_same_v<T, std::size_t>) {
    program->setUniformValue(location, GLint(value));
  } else if constexpr (std::is_same_v<T, Vec2f>) {
    program->setUniformValue(location, GLfloat(value.x), GLfloat(value.y));
  } else if constexpr (std::is_same_v<T, Vec2i>) {
    program->setUniformValue(location, GLint(value.x), GLint(value.y));
  } else if constexpr (std::is_same_v<T, bool>) {
    program->setUniformValue(location, GLboolean(value));
  } else if constexpr (std::is_same_v<T, QString>) {
    // string is not available in GLSL
  } else if constexpr (std::is_same_v<T, TriggerPropertyDummyValueType>) {
    // trigger is not available in GLSL
  } else if constexpr (std::is_same_v<T, SplineType>) {
    static constexpr std::size_t n = nodes::NodeCompilerGLSL::SPLINE_SIZE;
    const auto samples = sample<GLfloat>(value, n);
    program->setUniformValueArray(location, samples.data(), n, 1);
  } else if constexpr (std::is_same_v<T, ObjectTransformation>) {
    set_uniform(self, name, value.to_mat());
  } else if constexpr (std::is_same_v<T, Matrix>) {
    const auto mat = value.to_qmatrix3x3();
    program->setUniformValue(location, mat);
  } else {
    // statically fail here. If you're data type is not supported, add it explicitely.
    static_assert(std::is_same_v<T, int> && !std::is_same_v<T, int>);
    Q_UNIMPLEMENTED();
  }
}

}  // namespace

namespace omm
{
OffscreenRenderer::OffscreenRenderer()
{
  if (!m_context.create()) {
    LFATAL("Failed to create OpenGL Context.");
  }
  m_surface.create();
  m_context.makeCurrent(&m_surface);
  m_functions = m_context.functions();
  assert(m_functions != nullptr);

  QOpenGLVertexArrayObject::Binder vao_binder(&m_vao);

  m_vertices.create();
  m_vertices.bind();
  m_vertices.allocate(m_quad.data(), m_quad.size() * sizeof(decltype(m_quad)::value_type));

  m_functions->initializeOpenGLFunctions();
  m_functions->glEnableVertexAttribArray(0);
  m_functions->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
  m_vertices.release();

  static constexpr double CLEAR_COLOR_R = 1.0;
  static constexpr double CLEAR_COLOR_G = 0.5;
  static constexpr double CLEAR_COLOR_B = 0.0;
  static constexpr double CLEAR_COLOR_A = 1.0;
  glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
}

OffscreenRenderer::~OffscreenRenderer()
{
  // destroy the textures *before* m_context is destroyed.
  textures.clear();
}

bool OffscreenRenderer::set_fragment_shader(const QString& fragment_code)
{
  textures.clear();
  if (fragment_code.isEmpty()) {
    m_program.reset();
    return false;
  } else {
    m_program = std::make_unique<QOpenGLShaderProgram>();
    if (!m_context.makeCurrent(&m_surface)) {
      LERROR << "Failed to activate context.";
      return false;
    }
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_code)) {
      LERROR << "Failed to add vertex shader from source code.";
      return false;
    }
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_code)) {
      LERROR << "Failed to add fragment shader from source code.";
      return false;
    }
    m_program->bindAttributeLocation(vertex_position_attribute_name, 0);
    if (!m_program->link()) {
      LERROR << "Failed to link program.";
      return false;
    }
    if (!m_program->isLinked()) {
      LERROR << "Program is not linked.";
      return false;
    }
    return true;
  }
}

QOpenGLContext& OffscreenRenderer::context()
{
  return m_context;
}

QOpenGLShaderProgram* OffscreenRenderer::program() const
{
  return m_program.get();
}

void OffscreenRenderer::make_current()
{
  if (!m_context.makeCurrent(&m_surface)) {
    LFATAL("Failed to activate OpenGL context.");
  }
}

void OffscreenRenderer::set_uniform(const QString& name, const variant_type& value)
{
  std::visit([this, name](auto&& v) { ::set_uniform(*this, name, v); }, value);
}

std::unique_ptr<OffscreenRenderer> OffscreenRenderer::make()
{
  if (Application::instance().options().have_opengl) {
    return std::make_unique<OffscreenRenderer>();
  } else {
    return nullptr;
  }
}

Texture OffscreenRenderer::render(const Object& object,
                                  const QSize& size,
                                  const QRectF& roi,
                                  const PainterOptions& options)
{
  const QSize adjusted_size = QSize(static_cast<int>(size.width() * roi.width() / 2.0),
                                    static_cast<int>(size.height() * roi.height() / 2.0));
  if (m_program == nullptr) {
    return Texture(adjusted_size);
  } else if (adjusted_size.isEmpty()) {
    return Texture();
  }
  make_current();
  if (!m_context.isValid()) {
    LFATAL("OpenGL context is unexpectedly invalid.");
  }
  m_functions->glViewport(0, 0, adjusted_size.width(), adjusted_size.height());
  m_program->bind();
  const auto bb = object.bounding_box(ObjectTransformation());
  ::set_uniform(*this, "object_size", Vec2f(bb.width(), bb.height()));
  ::set_uniform(*this, "global_transform", object.global_transformation(Space::Scene));
  ::set_uniform(*this, "view_transform", object.global_transformation(Space::Viewport));
  ::set_uniform(*this, "view_size", Vec2f(options.device.width(), options.device.height()));
  ::set_uniform(*this, "roi_tl", Vec2f(roi.topLeft()));
  ::set_uniform(*this, "roi_br", Vec2f(roi.bottomRight()));
  ::set_uniform(*this, "object_id", options.object_id);
  ::set_uniform(*this, "path_id", options.path_id);

  m_vertices.bind();

  QOpenGLFramebufferObject fbo(adjusted_size);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  {
    QOpenGLVertexArrayObject::Binder vao_binder(&m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_quad.size());
  }
  m_program->release();
  m_vertices.release();
  m_vao.release();
  const QPoint offset(static_cast<int>((1.0 + roi.left()) / 2.0 * size.width()),
                      static_cast<int>((1.0 + roi.top()) / 2.0 * size.height()));
  return Texture(fbo.toImage(), offset);
}

}  // namespace omm
