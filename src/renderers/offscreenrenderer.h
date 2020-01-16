#pragma once

#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <memory>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include "logging.h"

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
  QOpenGLContext& context() { return m_context; }
  QOpenGLShaderProgram& program() const { return *m_program; }
  void make_current();

  template<typename T> void set_uniform(const QString& name, const T& value)
  {
    make_current();
    m_program->bind();
    m_program->setUniformValue(name.toStdString().c_str(), value);
  }

private:
  QOffscreenSurface m_surface;
  QOpenGLContext m_context;
  QOpenGLFunctions* m_functions;
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_vertices;
  std::unique_ptr<QOpenGLShaderProgram> m_program;
};

}  // namespace omm
