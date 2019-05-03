#include "mainwindow/exportdialog.h"
#include "widgets/referencelineedit.h"
#include <QPushButton>
#include "widgets/numericedit.h"
#include <QFrame>
#include "objects/view.h"
#include <QPainter>
#include "renderers/viewportrenderer.h"
#include "scene/scene.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include "geometry/vec2.h"

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

  connect(m_ui->ne_resolution_x, &AbstractNumericEdit::value_changed, [this]() {
    const auto ar = compute_aspect_ratio(view());
    m_ui->ne_resolution_y->set_value(static_cast<int>(m_ui->ne_resolution_x->value() / ar));
  });
  connect(m_ui->ne_resolution_y, &AbstractNumericEdit::value_changed, [this]() {
    const auto ar = compute_aspect_ratio(view());
    m_ui->ne_resolution_x->set_value(static_cast<int>(m_ui->ne_resolution_x->value() * ar));
  });

  update_preview();

  const int default_resolution_x = 1000;
  m_ui->ne_resolution_x->set_value(default_resolution_x);
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

  m_ui->lb_preview->setPixmap(QPixmap::fromImage(render(width, height)));
}

const View* ExportDialog::view() const
{
  return type_cast<View*>(kind_cast<Object*>(m_ui->cb_view->value()));
}

QImage ExportDialog::render(int width, int height) const
{
  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  ViewportRenderer renderer(m_scene, AbstractRenderer::Category::Objects);
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  renderer.set_painter(painter);
  const View* view = this->view();

  auto get_transformation = [width, view]() {
    if (view == nullptr) {
      const auto t = viewport().viewport_transformation();
      const auto s = width / double(viewport().width());
      return ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
    } else {
      const auto t = view->global_transformation(true).inverted();
      const auto view_size = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
      const auto s = width / double(view_size.x);
      const auto d = view_size/2.0;
      return ObjectTransformation().scaled(Vec2f(s, s)).apply(t.translated(d));
    }
  };

  m_scene.object_tree.root().set_transformation(get_transformation());

  m_scene.evaluate_tags();
  renderer.render();
  painter.end();

  return image;
}

void ExportDialog::save_as()
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
    if (render(m_ui->ne_resolution_x->value(), m_ui->ne_resolution_y->value()).save(filename)) {
      m_filepath = filename.toStdString();
    } else {
      const auto msg = tr("Writing image '%1' failed.").arg(filename);
      QMessageBox::warning(this, tr("Export image"), msg);
    }
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
  QDialog::showEvent(e);
}

}  // namespace omm
