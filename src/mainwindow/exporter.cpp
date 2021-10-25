#include "mainwindow/exporter.h"
#include "animation/animator.h"
#include "geometry/vec2.h"
#include "logging.h"
#include "mainwindow/viewport/viewport.h"
#include "objects/view.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/scene.h"
#include <fmt/format.h>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPaintDevice>
#include <QPicture>
#include <QSvgGenerator>
#include <QThread>

namespace omm
{
Exporter::Exporter(Scene& scene, const Viewport* viewport, QThread* thread)
    : m_scene(scene), m_viewport(viewport)
{
  if (thread != nullptr) {
    connect(thread, &QThread::started, this, &Exporter::start);
    connect(this, &Exporter::finished, thread, &QThread::quit);
    moveToThread(thread);
  }
}

void Exporter::start()
{
  Q_EMIT started();
  cancel = false;
  const auto& eo = export_options;
  if (eo.animated && filename(eo.start_frame) == filename(eo.end_frame)) {
    error_msg = tr("The pattern generates the same file name for multiple frames. "
                   "Insert the frame number placeholder {frame}.");
  } else {
    std::atomic<int> count = 0;
    const auto n = eo.end_frame - eo.start_frame + 1;
    Q_EMIT progress_changed(count, n);

    for (int frame = eo.start_frame; frame <= eo.end_frame; ++frame) {
      if (cancel) {
        break;
      }
      m_scene.animator().set_current(frame);
      render(frame, allow_overwrite);
      count += 1;
      Q_EMIT progress_changed(count, n);
    }
  }
  Q_EMIT finished();
}

QString Exporter::filename(int frame) const
{
  const QFileInfo scene_fi(m_scene.filename());
  const auto scene_path = scene_fi.dir().path();
  const auto scene_name = scene_fi.baseName();

  QString pattern = export_options.pattern;

  if (!QFileInfo(pattern).isAbsolute()) {
    pattern = scene_path + "/" + pattern;
  }
  if (!QFileInfo(pattern).isAbsolute()) {
    pattern = QApplication::applicationDirPath() + "/" + pattern;
  }

  return interpolate_filename(pattern, scene_path, scene_name, frame);
}

QString Exporter::interpolate_filename(const QString& pattern,
                                       const QString& scene_path,
                                       const QString& scene_name,
                                       int frame)
{
  const auto args = fmt::make_format_args(fmt::arg("path", scene_path.toStdString()),
                                          fmt::arg("name", scene_name.toStdString()),
                                          fmt::arg("frame", frame));
  const auto pattern_s = pattern.toStdString();
  return QString::fromStdString(fmt::vformat(pattern_s, args));
}

void Exporter::render(int frame, bool allow_overwrite)
{
  try {
    const QString filename = this->filename(frame);
    Q_EMIT post_status(tr("Writing %1 ...").arg(filename));
    const QFileInfo fi(filename);
    const auto dir = fi.dir();
    if (!dir.exists()) {
      if (!dir.mkpath(dir.path())) {
        LERROR << "Failed to create directory '" << dir.path() << "'.";
        post_status(tr("Failed to create directory '%1'.").arg(dir.path()));
      } else {
        LINFO << "Created directory '" << dir.path() << "'.";
      }
    }

    if (fi.exists() && !allow_overwrite) {
      LINFO << "Did not overwrite existing '" << filename << "'.";
      post_status(tr("No permission to overwrite existing file '%1'.").arg(filename));
    } else if (QFileInfo(fi.dir().path()).isWritable()) {
      const auto save_as = [this, filename]() {
        switch (export_options.format) {
        case ExportOptions::Format::Raster:
          return save_as_raster(filename);
        case ExportOptions::Format::Vector:
          return save_as_svg(filename);
        default:
          Q_UNREACHABLE();
        }
      };
      if (!save_as()) {
        LERROR << "Exporting frame " << frame << " failed.";
        Q_EMIT post_status(tr("Unexpected error."));
      }
    } else {
      LERROR << "Failed to write '" << filename << "'.";
      Q_EMIT post_status(tr("The filename '%1' cannot be written.").arg(filename));
    }
  } catch (const fmt::format_error& e) {
    Q_EMIT post_status(tr("The filename pattern is invalid."));
  }
}

View* Exporter::view() const
{
  auto* view = export_options.view;
  if (view == nullptr) {
    // The output view might have changed because it may be animated.
    // If no view is explicitely given, we want to respect that and broadcast a notification.
    for (auto* v : type_casts<View*>(m_scene.object_tree().items())) {
      if (v->property(View::OUTPUT_VIEW_PROPERTY_KEY)->value<bool>()) {
        view = v;
        break;
      }
    }
  }
  return view;
}

bool Exporter::save_as_raster(const QString& filename)
{
  QImage image({export_options.x_resolution, y_resolution}, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  render(image, 1.0);
  LINFO << "Wrote file '" << filename << "'.";
  return image.save(filename);
}

bool Exporter::save_as_svg(const QString& filename)
{
  QSvgGenerator generator;
  generator.setFileName(filename);
  if (export_options.view != nullptr) {
    auto view_box_size = export_options.view->property(View::SIZE_PROPERTY_KEY)->value<Vec2f>();
    view_box_size *= export_options.scale / view_box_size.x;
    generator.setViewBox(QRectF(0.0, 0.0, view_box_size.x, view_box_size.y));
  }
  render(generator, SVG_SCALE_FACTOR * export_options.scale);
  LINFO << "Wrote file '" << filename << "'.";
  return true;
}

void Exporter::render(QPaintDevice& device, double scale)
{
  QPicture picture;
  {
    QPainter painter(&picture);
    painter.setRenderHint(QPainter::Antialiasing);

    Painter renderer(m_scene, Painter::Category::Objects);
    renderer.painter = &painter;

    const auto transformation = [this, &device]() {
      auto* view = this->view();
      if (export_options.view == nullptr && view != nullptr) {
        Q_EMIT auto_view_changed(view);
      }

      if (view == nullptr && m_viewport == nullptr) {
        LWARNING << "View and viewport are invalid.";
        return ObjectTransformation{};
      } else if (view != nullptr) {
        const auto t = view->global_transformation(Space::Scene).inverted();
        const auto view_size = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
        const auto s = device.width() / double(view_size.x);
        const auto d = view_size / 2.0;
        return ObjectTransformation{}.scaled(Vec2f(s, s)).apply(t.translated(d));
      } else if (m_viewport != nullptr) {
        const auto t = m_viewport->viewport_transformation();
        const auto s = device.width() / double(m_viewport->width());
        return ObjectTransformation{}.scaled(Vec2f(s, s)).apply(t);
      } else {
        Q_UNREACHABLE();
      }
    }();

    m_scene.object_tree().root().set_transformation(transformation);
    m_scene.evaluate_tags();

    PainterOptions options(device);
    renderer.render(options);
  }

  QPainter final_painter(&device);

  final_painter.scale(scale, scale);
  final_painter.drawPicture(QPointF(0.0, 0.0), picture);
}

}  // namespace omm
