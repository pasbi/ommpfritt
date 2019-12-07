#include "offscreenrenderer.h"
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QImage>

namespace
{

static constexpr auto vertex_shader_source = R"(
attribute vec4 vertex;
varying vec3 vert;
void main() {
   vert = vertex.xyz;
   gl_Position = vertex * 20.0;
}
)";

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

void OffscreenRenderer::set_fragment_shader(const QString& fragment_code)
{
  m_program = std::make_unique<QOpenGLShaderProgram>();
  assert_or_call(m_context.makeCurrent(&m_surface));
  assert_or_call(m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source));
  assert_or_call(m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_code));
  m_program->bindAttributeLocation("vertex", 0);
  assert_or_call(m_program->link());
  assert(m_program->isLinked());
}

QImage OffscreenRenderer::render(const QSize& size)
{
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
