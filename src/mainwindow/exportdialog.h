#pragma once

#include <QDialog>
#include <memory>
#include "mainwindow/ui_exportdialog.h"
#include <QPicture>
#include <QImage>

class QPushButton;
class QLabel;

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

private:
  Scene& m_scene;
  QLabel* m_preview_label;
  ReferenceLineEdit* m_view_combobox;
  QPushButton* m_export_button;
  NumericEdit<int>* m_resolution_x;
  NumericEdit<int>* m_resolution_y;

  void save_as();
  void update_preview();
  const View* view() const;
  QImage render(int width, int height) const;
  QPicture m_picture;
  std::string m_filepath;

};

}  // namespace
