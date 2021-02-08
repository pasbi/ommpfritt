#pragma once

#include "exportoptions.h"
#include <QObject>
#include <QSize>
#include <set>

class QPaintDevice;

namespace omm
{
class Scene;
class View;
class Viewport;

class Exporter : public QObject
{
  Q_OBJECT

public:
  explicit Exporter(Scene& scene, const Viewport* viewport = nullptr, QThread* thread = nullptr);

  bool allow_overwrite = false;
  ExportOptions export_options;
  int y_resolution = 1024;

  static constexpr double SVG_SCALE_FACTOR = -4.0 / 3.0;
  static QString interpolate_filename(const QString& pattern,
                                      const QString& scene_path,
                                      const QString& scene_name,
                                      int frame);
  enum class Status { Waiting, Done, IOError };
  std::map<int, Status> status;

  QString error_msg;
  bool cancel = false;
  bool save_as_svg(const QString& filename);
  bool save_as_raster(const QString& filename);
  void render(QPaintDevice& device, double scale);
  View* view() const;

private:
  Scene& m_scene;
  const Viewport* const m_viewport;
  [[nodiscard]] QString filename(int frame) const;
  void render(int frame, bool allow_overwrite);

public:
  void start();

Q_SIGNALS:
  void started();
  void post_status(const QString& msg);
  void progress_changed(int current, int total);
  void finished();
  void auto_view_changed(omm::View* view);
};

}  // namespace omm
