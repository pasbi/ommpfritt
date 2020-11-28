#pragma once

#include "aspects/abstractpropertyowner.h"
#include "logging.h"
#include "renderers/painter.h"
#include "renderers/texture.h"
#include "variant.h"
#include <QOffscreenSurface>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <memory>

class QOpenGLFunctions;
class QOpenGLShaderProgram;
class QOpenGLTexture;

namespace omm
{
class OffscreenRenderer
{
public:
  OffscreenRenderer();
  ~OffscreenRenderer();
  Texture render(const Object& object,
                 const QSize& size,
                 const QRectF& roi,
                 const Painter::Options& options);
  bool set_fragment_shader(const QString& fragment_code);
  QOpenGLContext& context()
  {
    return m_context;
  }

  [[nodiscard]] QOpenGLShaderProgram* program() const
  {
    return m_program.get();
  }

  void make_current();

  void set_uniform(const QString& name, const variant_type& value);

  struct ShaderInput {
    enum class Kind { Uniform, Varying };
    const QString type;
    const char* name;
    const Kind kind;
    [[nodiscard]] QString tr_name() const;
  };

  static const std::vector<ShaderInput> fragment_shader_inputs;
  struct GLTexture {
    GLuint layout_location;
    std::unique_ptr<QOpenGLTexture> texture;
  };

  std::map<GLuint, GLTexture> textures;
  static std::unique_ptr<OffscreenRenderer> make();

private:
  QOffscreenSurface m_surface;
  QOpenGLContext m_context;
  QOpenGLFunctions* m_functions;
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_vertices;
  std::unique_ptr<QOpenGLShaderProgram> m_program;
};

}  // namespace omm
