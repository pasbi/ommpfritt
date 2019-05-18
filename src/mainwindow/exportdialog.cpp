#include "mainwindow/exportdialog.h"
#include "widgets/referencelineedit.h"
#include <QPushButton>
#include "widgets/numericedit.h"
#include <QFrame>
#include "objects/view.h"
#include <QPainter>
#include <QSettings>
#include "renderers/painter.h"
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QtSvg/QSvgGenerator>
#include "geometry/vec2.h"
#include "ui_exportdialog.h"

namespace
{

auto make_resolution_edit()
{
  auto edit = std::make_unique<omm::NumericEdit<int>>();
  edit->set_range(0, 100000);
  return edit;
}

class XLabel : public QLabel
{
public:
  XLabel() : QLabel("x") { setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred); }
};

class Line : public QFrame
{
public:
  Line(QFrame::Shape shape) : QFrame() { setFrameShape(shape); setFrameShadow(QFrame::Sunken); }
};

omm::Viewport& viewport() { return omm::Application::instance().main_window()->viewport(); }

double compute_aspect_ratio(const omm::View* view)
{
  if (view == nullptr) {
    return double(viewport().width()) / double(viewport().height());
  } else {
    const omm::Vec2f s = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
    return s.x / s.y;
  }
}

}

namespace omm
{

ExportDialog::ExportDialog(Scene& scene, QWidget* parent)
  : QDialog(parent), m_scene(scene)
  , m_filepath(scene.filename())
  , m_ui(new ::Ui::ExportDialog)
{
  m_ui->setupUi(this);

  QObject::connect(m_ui->cb_view, &ReferenceLineEdit::value_changed, [this]() {
    m_ui->ne_resolution_y->set_value(int(m_ui->ne_resolution_x->value()
                                         / compute_aspect_ratio(view())));
    update_preview();
  });

  m_ui->cb_view->set_filter(AbstractPropertyOwner::Kind::Object);
  m_ui->cb_view->set_filter(AbstractPropertyOwner::Flag::IsView);
  m_ui->cb_view->set_null_label(tr("Viewport").toStdString());
  m_ui->cb_view->set_scene(scene);

  connect(m_ui->pb_export, &QPushButton::clicked, this, &ExportDialog::save_as);

  const auto update_resolution_y_edit = [this]() {
    const auto ar = compute_aspect_ratio(view());
    m_ui->ne_resolution_y->set_value(static_cast<int>(m_ui->ne_resolution_x->value() / ar));
  };
  const auto update_resolution_x_edit = [this]() {
    const auto ar = compute_aspect_ratio(view());
    m_ui->ne_resolution_x->set_value(static_cast<int>(m_ui->ne_resolution_x->value() * ar));
  };
  connect(m_ui->ne_resolution_y, &AbstractNumericEdit::value_changed, update_resolution_x_edit);
  connect(m_ui->ne_resolution_x, &AbstractNumericEdit::value_changed, update_resolution_y_edit);

  update_preview();

  const int default_resolution_x = 1000;
  m_ui->ne_resolution_x->set_value(default_resolution_x);
  update_resolution_y_edit();

  QSettings settings;
  m_ui->cb_format->setCurrentIndex(settings.value(FORMAT_SETTINGS_KEY, 0).toInt());
  update_active_view();

  m_ui->ne_scaling->set_value(100.0);
  m_ui->ne_scaling->set_step(0.01);
}

void ExportDialog::update_preview()
{
  int width = m_ui->lb_preview->width();
  int height = m_ui->lb_preview->height();

  if (const double ar = compute_aspect_ratio(view()); double(width) / double(height) > ar) {
    width = static_cast<int>(height * ar);
  } else {
    height = static_cast<int>(width / ar);
  }

  m_ui->lb_preview->setPixmap(QPixmap::fromImage(rasterize(width, height)));
}

const View* ExportDialog::view() const
{
  return type_cast<View*>(kind_cast<Object*>(m_ui->cb_view->value()));
}

QImage ExportDialog::rasterize(int width, int height) const
{
  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  render(image);
  return image;
}

void ExportDialog::render(QPaintDevice& device, double scale) const
{
  QPicture picture;
  {
    QPainter painter(&picture);
    painter.setRenderHint(QPainter::Antialiasing);

    Painter renderer(m_scene, Painter::Category::Objects);
    renderer.painter = &painter;

    auto get_transformation = [&device, view=this->view()]() {
      if (view == nullptr) {
        const auto t = viewport().viewport_transformation();
        const auto s = device.width() / double(viewport().width());
        return ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
      } else {
        const auto t = view->global_transformation(true).inverted();
        const auto view_size = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
        const auto s = device.width() / double(view_size.x);
        const auto d = view_size/2.0;
        return ObjectTransformation().scaled(Vec2f(s, s)).apply(t.translated(d));
      }
    };

    m_scene.object_tree.root().set_transformation(get_transformation());

    m_scene.evaluate_tags();
    renderer.render();
  }

  QPainter final_painter(&device);
  final_painter.scale(scale, scale);
  final_painter.drawPicture(QPointF(0.0, 0.0), picture);
}

void ExportDialog::save_as_raster()
{
  QFileDialog file_dialog(this);
  file_dialog.setWindowTitle(tr("Export image ..."));
  file_dialog.setDefaultSuffix("png");
  file_dialog.setDirectory(QString::fromStdString(m_filepath));
  file_dialog.setNameFilter(tr("Images (*.png *.jpg)"));
  file_dialog.setFileMode(QFileDialog::AnyFile);
  file_dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (file_dialog.exec() == QDialog::Accepted) {
    const auto filenames = file_dialog.selectedFiles();
    assert(filenames.size() == 1);
    const auto filename = filenames.front();
    if (rasterize(m_ui->ne_resolution_x->value(), m_ui->ne_resolution_y->value()).save(filename)) {
      m_filepath = filename.toStdString();
    } else {
      const auto msg = tr("Writing image '%1' failed.").arg(filename);
      QMessageBox::warning(this, tr("Export image"), msg);
    }
  }
}

void ExportDialog::save_as_svg()
{
  QFileDialog file_dialog(this);
  file_dialog.setWindowTitle(tr("Export vector graphics..."));
  file_dialog.setDefaultSuffix("svg");
  file_dialog.setDirectory(QString::fromStdString(m_filepath));
  file_dialog.setNameFilter(tr("SVG (*.svg)"));
  file_dialog.setFileMode(QFileDialog::AnyFile);
  file_dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (file_dialog.exec() == QDialog::Accepted) {
    const auto filenames = file_dialog.selectedFiles();
    assert(filenames.size() == 1);
    const auto filename = filenames.front();

    QByteArray buffer;
    QSvgGenerator generator;
    generator.setFileName(filename);
    const auto scale = m_ui->ne_scaling->value();
    auto view_box_size = view()->property(View::SIZE_PROPERTY_KEY)->value<Vec2f>();
    view_box_size *= scale / view_box_size.x;
    generator.setViewBox(QRectF(0.0, 0.0, view_box_size.x, view_box_size.y));
    render(generator, -scale * 4.0 / 3.0);
    m_filepath = filename.toStdString();
  }
}

void ExportDialog::update_active_view()
{
  m_ui->cb_view->update_candidates();
  for (auto* view : type_cast<View*>(m_scene.object_tree.items())) {
    if (view->property(View::OUTPUT_VIEW_PROPERTY_KEY)->value<bool>()) {
      m_ui->cb_view->set_value(view);
      break;
    }
  }
}

void ExportDialog::save_settings()
{
  QSettings settings;
  settings.setValue(FORMAT_SETTINGS_KEY, m_ui->cb_format->currentIndex());
}

void ExportDialog::save_as()
{
  switch (m_ui->cb_format->currentIndex()) {
  case 0: save_as_raster(); break;
  case 1: save_as_svg(); break;
  }

}

void ExportDialog::resizeEvent(QResizeEvent* e)
{
  update_preview();
  QDialog::resizeEvent(e);
}

void ExportDialog::showEvent(QShowEvent* e)
{
  update_preview();
  update_active_view();
  QDialog::showEvent(e);
}

void ExportDialog::hideEvent(QHideEvent *e)
{
  save_settings();
  QDialog::hideEvent(e);
}

void ExportDialog::UiExportDialogDeleter::operator()(Ui::ExportDialog *ui)
{
  delete ui;
}

}  // namespace omm
