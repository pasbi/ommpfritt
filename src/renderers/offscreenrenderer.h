#pragma once

#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <memory>
#include <QOffscreenSurface>

class QOpenGLFunctions;
class QOpenGLShaderProgram;

namespace omm
{

class OffscreenRenderer
{
public:
  OffscreenRenderer();
  ~OffscreenRenderer();
  QImage render(const QSize& size);
  void set_fragment_shader(const QString& fragment_code);

private:
  QOffscreenSurface m_surface;
  QOpenGLContext m_context;
  QOpenGLFunctions* m_functions;
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_vertices;
  std::unique_ptr<QOpenGLShaderProgram> m_program;
};

}  // namespace omm
