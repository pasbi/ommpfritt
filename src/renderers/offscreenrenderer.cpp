#include "offscreenrenderer.h"
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QImage>
#include "managers/nodemanager/nodecompilerglsl.h"
#include "geometry/objecttransformation.h"
#include <QApplication>
#include "objects/object.h"

namespace omm
{

const std::vector<OffscreenRenderer::VaryingInfo> OffscreenRenderer::varyings = {
  {
    NodeCompilerTypes::FLOATVECTOR_TYPE,
    QT_TRANSLATE_NOOP("OffscreenRenderer", "local_pos")
  },
  {
    NodeCompilerTypes::FLOATVECTOR_TYPE,
    QT_TRANSLATE_NOOP("OffscreenRenderer", "global_pos")
  },
  {
    NodeCompilerTypes::FLOATVECTOR_TYPE,
    QT_TRANSLATE_NOOP("OffscreenRenderer", "local_normalized_pos")
  },
  {
    NodeCompilerTypes::FLOATVECTOR_TYPE,
    QT_TRANSLATE_NOOP("OffscreenRenderer", "size")
  },
  {
    NodeCompilerTypes::FLOATVECTOR_TYPE,
    QT_TRANSLATE_NOOP("OffscreenRenderer", "view_pos")
  },
};

QString OffscreenRenderer::VaryingInfo::tr_name() const
{
  return QApplication::translate("OffscreenRenderer", name);
}

}  // namespace omm

namespace
{

static constexpr auto vertex_position_attribute_name = "vertex_attr";

using S = omm::OffscreenRenderer;
static constexpr auto vertex_code = R"(
#version 330

attribute vec4 vertex_attr;
varying vec2 local_pos;
varying vec2 local_normalized_pos;
varying vec2 global_pos;
varying vec2 view_pos;
varying vec2 size;
uniform vec2 top_left;
uniform vec2 bottom_right;
uniform mat3 global_transform;
uniform mat3 view_transform;
uniform vec2 view_size;

vec2 unc(vec2 centered) {
    return (centered + vec2(1.0, 1.0)) / 2.0;
}

void main() {
  vec2 local_normalized_centered_pos = vertex_attr.xy * vec2(1.0, -1.0);
  size = bottom_right - top_left;
  vec2 local_centered_pos = local_normalized_centered_pos * size / 2.0;
  global_pos = (global_transform * vec3(local_centered_pos, 1.0)).xy;
  view_pos = (view_transform * vec3(local_centered_pos, 1.0)).xy / view_size;
  local_pos = unc(local_normalized_centered_pos) * size / 2.0;
  local_normalized_pos = unc(local_normalized_centered_pos);
  gl_Position = vec4(vertex_attr.xy, 0.0, 1.0);
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
  const auto ba = name.toUtf8();
  const char* cname = ba.constData();
  assert(QString(cname) == name);
  if constexpr (std::is_same_v<T, double>) {
    program->setUniformValue(cname, GLfloat(value));
  } else if constexpr (std::is_same_v<T, int>) {
    program->setUniformValue(cname, value);
  } else if constexpr (std::is_same_v<T, AbstractPropertyOwner*>) {
    if (value == nullptr) {
      program->setUniformValue(cname, 0);
    } else {
      program->setUniformValue(cname, GLuint(value->id()));
    }
  } else if constexpr (std::is_same_v<T, Color>) {
    auto [r, g, b, a] = value.components(Color::Model::RGBA);
    program->setUniformValue(cname, QVector4D(r, g, b, a));
  } else if constexpr (std::is_same_v<T, std::size_t>) {
    program->setUniformValue(cname, GLint(value));
  } else if constexpr (std::is_same_v<T, Vec2f>) {
    program->setUniformValue(cname, GLfloat(value.x), GLfloat(value.y));
  } else if constexpr (std::is_same_v<T, Vec2i>) {
    program->setUniformValue(cname, GLint(value.x), GLint(value.y));
  } else if constexpr (std::is_same_v<T, bool>) {
    program->setUniformValue(cname, GLboolean(value));
  } else if constexpr (std::is_same_v<T, QString>) {
    // string is not available in GLSL
  } else if constexpr (std::is_same_v<T, TriggerPropertyDummyValueType>) {
    // string is not available in GLSL
  } else if constexpr (std::is_same_v<T, ObjectTransformation>) {
    set_uniform(self, name, value.to_mat());
  } else if constexpr (std::is_same_v<T, Matrix>) {
    const auto mat = value.to_qmatrix3x3();
    program->setUniformValue(cname, mat);
  } else {
    // statically fail here. If you're data type is not supported, add it explicitely.
    static_assert(std::is_same_v<T, int> && !std::is_same_v<T, int>);
    Q_UNIMPLEMENTED();
  }
}

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

//    QStringList lines = fragment_code.split("\n");
//    for (int i = 0; i < lines.size(); ++i) {
//      lines[i] = QString("%1 %2").arg(i+1, log(lines.size()+1)/log(10) + 1).arg(lines[i]);
//    }
//    LINFO << "code:\n" << lines.join("\n");

    m_program = std::make_unique<QOpenGLShaderProgram>();
    CHECK(m_context.makeCurrent(&m_surface));
    CHECK(m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_code));
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
  std::visit([this, name](auto&& v) { ::set_uniform(*this, name, v); }, value);
}

QImage OffscreenRenderer::render(const Object& object, const QSize& size,
                                 const Painter::Options& options)
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
  const auto bb = object.bounding_box(ObjectTransformation());
  ::set_uniform(*this, "top_left", Vec2f(bb.left(), bb.top()));
  ::set_uniform(*this, "bottom_right", Vec2f(bb.right(), bb.bottom()));
  ::set_uniform(*this, "global_transform", object.global_transformation(Space::Scene));
  ::set_uniform(*this, "view_transform", object.global_transformation(Space::Viewport));
  ::set_uniform(*this, "view_size", Vec2f(options.device.width(), options.device.height()));

  m_vertices.bind();

  QOpenGLFramebufferObject fbo(size);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
