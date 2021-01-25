#pragma once

#include "mainwindow/exportoptions.h"
#include <QAbstractListModel>
#include <QDialog>
#include <QImage>
#include <QPicture>
#include <QThread>
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
class FilenamePatternValidator;
class Exporter;

class ExportDialog : public QDialog
{
  Q_OBJECT
private:
  class MapListModel : public QAbstractListModel
  {
  public:
    using map_type = std::vector<std::pair<QString, QString>>;
    explicit MapListModel(const map_type& values);
    const std::vector<QString> labels;
    const std::vector<QString> codes;
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount([[maybe_unused]] const QModelIndex& index) const override;
  };

public:
  ExportDialog(Scene& scene, QWidget* parent);
  ~ExportDialog() override;
  ExportDialog(ExportDialog&&) = delete;
  ExportDialog(const ExportDialog&) = delete;
  ExportDialog& operator=(ExportDialog&&) = delete;
  ExportDialog& operator=(const ExportDialog&) = delete;

protected:
  void resizeEvent(QResizeEvent* e) override;
  void showEvent(QShowEvent* e) override;
  void hideEvent(QHideEvent* e) override;
  void closeEvent(QCloseEvent* e) override;

private:
  Scene& m_scene;

  void update_preview();

  std::unique_ptr<::Ui::ExportDialog> m_ui;
  std::unique_ptr<FilenamePatternValidator> m_validator;
  std::unique_ptr<MapListModel> m_raster_format_list_model;
  std::unique_ptr<MapListModel> m_svg_format_list_model;
  std::unique_ptr<MapListModel> m_variable_list_model;
  QThread m_exporter_thread;
  std::unique_ptr<Exporter> m_exporter;

  void save_settings();
  void restore_settings();
  void set_default_values();
  void connect_gui();
  void set_animation_range(int start, int end);
  void update_frame_range_limits();
  void update_view_null_label(const View* view);
  ExportOptions export_options() const;
  bool m_options_are_plausible = false;
  void update_enabledness(bool job_running);
  double compute_aspect_ratio() const;
  void update_exporter();

private:
  void update_pattern_edit_background();
  void update_ending_cb();
  void reset_start_frame();
  void reset_end_frame();
  void start_export();
  void update_y_edit();
  void update_x_edit();
};

}  // namespace omm
