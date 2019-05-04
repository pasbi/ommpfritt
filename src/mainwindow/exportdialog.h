#pragma once

#include <QDialog>
#include <memory>
#include <QPicture>
#include <QImage>

class QPushButton;
class QLabel;

namespace Ui { class ExportDialog; }

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

protected:
  void resizeEvent(QResizeEvent* e) override;
  void showEvent(QShowEvent* e) override;
  void hideEvent(QHideEvent* e) override;

private:
  Scene& m_scene;

  void save_as();
  void update_preview();
  const View* view() const;
  void render(QPaintDevice& device, double scale = 1.0) const;
  QImage rasterize(int width, int height) const;
  void save_as_raster();
  void save_as_svg();
  QPicture m_picture;
  std::string m_filepath;

  struct UiExportDialogDeleter
  {
    void operator()(::Ui::ExportDialog* ui);
  };

  std::unique_ptr<::Ui::ExportDialog, UiExportDialogDeleter> m_ui;

  static constexpr auto FORMAT_SETTINGS_KEY = "last_format";
  void update_active_view();
  void save_settings();
};

}  // namespace
