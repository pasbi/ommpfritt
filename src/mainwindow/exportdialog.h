#pragma once

#include <QDialog>
#include <memory>
#include <QPicture>
#include <QImage>
#include "ui_exportdialog.h"

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

  void save_as();
  void update_preview();
  const View* view() const;
  QImage render(int width, int height) const;
  QPicture m_picture;
  std::string m_filepath;

  std::unique_ptr<::Ui::ExportDialog> m_ui;

};

}  // namespace
