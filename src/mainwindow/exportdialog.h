#pragma once

#include <QDialog>
#include <QImage>
#include <QPicture>
#include <memory>

class QPushButton;
class QLabel;
class QValidator;

namespace Ui
{
class ExportDialog;
}

namespace omm
{
class Scene;
template<typename> class NumericEdit;
class ReferenceLineEdit;
class View;

class ExportDialog : public QDialog
{
  Q_OBJECT
public:
  ExportDialog(Scene& scene, QWidget* parent);
  ~ExportDialog() override;

  static void render(Scene& scene, const View* view, QPaintDevice& device, double scale = 1.0);

protected:
  void resizeEvent(QResizeEvent* e) override;
  void showEvent(QShowEvent* e) override;
  void hideEvent(QHideEvent* e) override;

private:
  Scene& m_scene;

  void save_as();
  void update_preview();
  [[nodiscard]] const View* view() const;
  void save_as_raster();
  void save_as_svg();
  QPicture m_picture;
  QString m_filepath;
  QString m_animation_directory;
  static QString filename(QString pattern, int frame_number);

  std::unique_ptr<::Ui::ExportDialog> m_ui;
  std::unique_ptr<QValidator> m_validator;

  static constexpr auto FORMAT_SETTINGS_KEY = "last_format";
  void update_active_view();
  void save_settings();

private Q_SLOTS:
  void update_pattern_edit_background();
  void reset_start_frame();
  void reset_end_frame();
  void set_maximum_start(int max);
  void set_minimum_end(int min);
  void start_export_animation();
};

}  // namespace omm
