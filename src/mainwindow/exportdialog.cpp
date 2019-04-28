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

auto make_resolution_edit(const omm::NumericEdit<int>::on_value_changed_t& ovt)
{
  auto edit = std::make_unique<omm::NumericEdit<int>>(ovt);
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
{
  auto preview_label = std::make_unique<QLabel>();
  m_preview_label = preview_label.get();
  m_preview_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  m_preview_label->setMinimumSize(100, 100);
  m_preview_label->setAlignment(Qt::AlignCenter);
  auto view_combobox = std::make_unique<ReferenceLineEdit>(scene, [this](auto*) {
    m_resolution_y->set_value(int(m_resolution_x->value() / compute_aspect_ratio(view())));
    update_preview();
  });

  m_view_combobox = view_combobox.get();
  m_view_combobox->set_filter(AbstractPropertyOwner::Kind::Object);
  m_view_combobox->set_filter(AbstractPropertyOwner::Flag::IsView);
  m_view_combobox->set_null_label(tr("Viewport").toStdString());

  const auto update_resolution_x = [this](int y) {
    m_resolution_x->set_value(int(y * compute_aspect_ratio(view())));
  };

  const auto update_resolution_y = [this](int x) {
    m_resolution_y->set_value(int(x / compute_aspect_ratio(view())));
  };

  auto export_button = std::make_unique<QPushButton>(tr("Save as ..."));
  m_export_button = export_button.get();

  connect(m_export_button, &QPushButton::clicked, this, &ExportDialog::save_as);

  auto resolution_x = make_resolution_edit(update_resolution_y);
  m_resolution_x = resolution_x.get();
  auto resolution_y = make_resolution_edit(update_resolution_x);
  m_resolution_y = resolution_y.get();

  auto resolution_layout = std::make_unique<QHBoxLayout>();
  resolution_layout->addWidget(resolution_x.release());
  resolution_layout->addWidget(std::make_unique<XLabel>().release());
  resolution_layout->addWidget(resolution_y.release());

  auto column_layout = std::make_unique<QVBoxLayout>();
  column_layout->addWidget(view_combobox.release());
  column_layout->addLayout(resolution_layout.release());
  column_layout->addWidget(export_button.release());
  column_layout->addStretch();

  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(preview_label.release());
  layout->addWidget(std::make_unique<Line>(QFrame::HLine).release());
  layout->addLayout(column_layout.release());
  layout->setStretch(0, 1);
  layout->setStretch(1, 0);
  layout->setStretch(2, 0);
  setLayout(layout.release());

  update_preview();

  const int default_resolution_x = 1000;
  m_resolution_x->set_value(default_resolution_x);
  update_resolution_y(default_resolution_x);
}

void ExportDialog::update_preview()
{
  int width = m_preview_label->width();
  int height = m_preview_label->height();

  if (const double ar = compute_aspect_ratio(view()); double(width) / double(height) > ar) {
    width = static_cast<int>(height * ar);
  } else {
    height = static_cast<int>(width / ar);
  }

  m_preview_label->setPixmap(QPixmap::fromImage(render(width, height)));
}

const View* ExportDialog::view() const
{
  return type_cast<View*>(kind_cast<Object*>(m_view_combobox->value()));
}

QImage ExportDialog::render(int width, int height) const
{
  QImage image(width, height, QImage::Format_RGB888);
  image.fill(Qt::white);
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
    if (render(m_resolution_x->value(), m_resolution_y->value()).save(filename)) {
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
