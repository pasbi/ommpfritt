#include "offscreenrenderer.h"
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QImage>

namespace
{

static constexpr auto vertex_position_attribute_name = "vertex_attr";

static const auto vertex_shader_source = QString(R"(
attribute vec4 %1;
varying vec2 %2;
void main() {
   %2 = %1.xy;
   gl_Position = %1 * 20.0;
}
)").arg(vertex_position_attribute_name).arg(omm::OffscreenRenderer::vertex_position_name);

static constexpr std::array<float, 18> m_quad = {
  -1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
  -1.0,  1.0, 0.0,

  -1.0, -1.0, 0.0,
   1.0, -1.0, 0.0,
   1.0,  1.0, 0.0,
};

}  // namespace

#ifdef NDEBUG
# define assert_or_call(expr) (expr)
#else
# define assert_or_call(expr) assert(expr)
#endif

namespace omm
{

OffscreenRenderer::OffscreenRenderer()
{
  assert_or_call(m_context.create());
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
  m_functions->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  m_vertices.release();

  glClearColor(1.0, 0.5, 0, 1.0);
}

OffscreenRenderer::~OffscreenRenderer()
{
}

bool OffscreenRenderer::set_fragment_shader(const QString& fragment_code)
{
  if (fragment_code.isEmpty()) {
    m_program.reset();
    return false;
  } else {
#define CHECK(X) if (!(X)) { LERROR << #X" failed."; return false; }

    QStringList lines = fragment_code.split("\n");
    for (int i = 0; i < lines.size(); ++i) {
      lines[i] = QString("%1 %2").arg(i+1, log(lines.size()+1)/log(10) + 1).arg(lines[i]);
    }
    LINFO << "code:\n" << lines.join("\n");

    m_program = std::make_unique<QOpenGLShaderProgram>();
    CHECK(m_context.makeCurrent(&m_surface));
    CHECK(m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source));
    CHECK(m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_code));
    m_program->bindAttributeLocation(vertex_position_attribute_name, 0);
    CHECK(m_program->link());
    CHECK(m_program->isLinked());
    return true;
#undef CHECK
  }

}

void OffscreenRenderer::make_current()
{
  assert_or_call(m_context.makeCurrent(&m_surface));
}

void OffscreenRenderer::set_uniform(const QString& name, const variant_type& value)
{
  make_current();
  if (m_program == nullptr) {
    // if the shader has never been built successfully, `m_program` will be null.
    return;
  }
  m_program->bind();
  const char* cname = name.toStdString().c_str();
  std::visit([program=m_program.get(), cname](auto&& v) {
    using V = std::decay_t<decltype(v)>;
    if constexpr (std::is_same_v<V, double>) {
      program->setUniformValue(cname, GLfloat(v));
    } else if constexpr (std::is_same_v<V, AbstractPropertyOwner*>) {
      if (v == nullptr) {
        program->setUniformValue(cname, 0);
      } else {
        program->setUniformValue(cname, GLuint(v->id()));
      }
    } else if constexpr (std::is_same_v<V, Color>) {
      auto [r, g, b, a] = v.components(Color::Model::RGBA);
      program->setUniformValue(cname, QVector4D(r, g, b, a));
    } else if constexpr (std::is_same_v<V, std::size_t>) {
      program->setUniformValue(cname, GLint(v));
    } else if constexpr (std::is_same_v<V, Vec2f>) {
      program->setUniformValue(cname, GLfloat(v.x), GLfloat(v.y));
    } else if constexpr (std::is_same_v<V, Vec2i>) {
      program->setUniformValue(cname, GLint(v.x), GLint(v.y));
    } else if constexpr (std::is_same_v<V, bool>) {
      program->setUniformValue(cname, GLboolean(v));
    } else if constexpr (std::is_same_v<V, QString>) {
      // string is not available in GLSL
    } else if constexpr (std::is_same_v<V, TriggerPropertyDummyValueType>) {
      // string is not available in GLSL
    } else {
      // statically fail here.
      static_assert(std::is_same_v<V, int>);
      Q_UNIMPLEMENTED();
    }
  }, value);
}

QImage OffscreenRenderer::render(const QSize& size)
{
  if (m_program == nullptr) {
    return QImage(size, QImage::Format_ARGB32_Premultiplied);
  }
  assert_or_call(m_context.makeCurrent(&m_surface));
  assert_or_call(m_context.isValid());
  QOpenGLFunctions* functions = m_context.functions();
  assert (functions != nullptr);

  m_functions->glViewport(0, 0, size.width(), size.height());
  m_program->bind();

  QOpenGLFramebufferObject fbo(size);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_program->bind();
  {
    QOpenGLVertexArrayObject::Binder vao_binder(&m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_quad.size());
  }
  m_program->release();
  m_vertices.release();
  m_vao.release();
  return fbo.toImage();
}

}  // namespace omm
