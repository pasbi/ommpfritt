#include "mainwindow/exportdialog.h"
#include "animation/animator.h"
#include "geometry/vec2.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "objects/view.h"
#include "renderers/painter.h"
#include "scene/scene.h"
#include "ui_exportdialog.h"
#include "widgets/numericedit.h"
#include "widgets/referencelineedit.h"
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>
#include <QtSvg/QSvgGenerator>

namespace
{
omm::Viewport& viewport()
{
  return omm::Application::instance().main_window()->viewport();
}

double compute_aspect_ratio(const omm::View* view)
{
  if (view == nullptr) {
    return double(viewport().width()) / double(viewport().height());
  } else {
    const omm::Vec2f s = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
    return s.x / s.y;
  }
}

const QChar frame_number_placeholder = '%';

class FilenamePatternValidator : public QValidator
{
public:
  QValidator::State validate(QString& input, int&) const override
  {
    const std::set<QString> forbidden_subsequences = {"/"};
    const std::set<QString> allowed_endings = {".png", ".jpg", ".jpeg"};

    for (const QString& forbidden_subsequence : forbidden_subsequences) {
      if (input.contains(forbidden_subsequence)) {
        return QValidator::Invalid;
      }
    }

    const int n = input.count(frame_number_placeholder);
    if (n == 0) {
      return QValidator::Intermediate;
    }
    const int first_occurence = input.indexOf(frame_number_placeholder);
    const int last_occurence = input.lastIndexOf(frame_number_placeholder);
    if (last_occurence - first_occurence + 1 != n) {
      return QValidator::Invalid;
    }

    for (const QString& allowed_ending : allowed_endings) {
      if (input.endsWith(allowed_ending, Qt::CaseInsensitive)) {
        return QValidator::Acceptable;
      }
    }
    return QValidator::Intermediate;
  }
};

void update_edit(omm::IntNumericEdit& primary, omm::IntNumericEdit& secondary, double aspect_ratio)
{
  const int p_value = primary.value();
  const int s_value = static_cast<int>(p_value * aspect_ratio);
  QSignalBlocker blocker(secondary);
  secondary.set_value(s_value);
}

}  // namespace

namespace omm
{
ExportDialog::ExportDialog(Scene& scene, QWidget* parent)
    : QDialog(parent), m_scene(scene), m_filepath(scene.filename()),
      m_animation_directory(QFileInfo(m_filepath).dir().path()), m_ui(new ::Ui::ExportDialog),
      m_validator(new FilenamePatternValidator())
{
  m_ui->setupUi(this);
  set_default_values(scene);
  connect_gui();
  update_y_edit();
  update_preview();
  update_active_view();
  update_pattern_edit_background();
  reset_start_frame();
  reset_end_frame();
  restore_settings();
}

ExportDialog::~ExportDialog() = default;

void ExportDialog::update_preview()
{
  int width = m_ui->lb_preview->width();
  int height = m_ui->lb_preview->height();

  if (const double ar = compute_aspect_ratio(view()); double(width) / double(height) > ar) {
    width = static_cast<int>(height * ar);
  } else {
    height = static_cast<int>(width / ar);
  }

  QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
  {
    QPainter painter(&image);
    UiColors::draw_background(painter, image.rect());
  }
  render(m_scene, view(), image);
  m_ui->lb_preview->setPixmap(QPixmap::fromImage(image));
}

const View* ExportDialog::view() const
{
  return type_cast<View*>(kind_cast<Object*>(m_ui->cb_view->value()));
}

void ExportDialog::render(Scene& scene, const View* view, QPaintDevice& device, double scale)
{
  QPicture picture;
  {
    QPainter painter(&picture);
    painter.setRenderHint(QPainter::Antialiasing);

    Painter renderer(scene, Painter::Category::Objects);
    renderer.painter = &painter;

    const auto transformation = [&device, view]() {
      if (view == nullptr) {
        const auto t = viewport().viewport_transformation();
        const auto s = device.width() / double(viewport().width());
        return ObjectTransformation().scaled(Vec2f(s, s)).apply(t);
      } else {
        const auto t = view->global_transformation(Space::Scene).inverted();
        const auto view_size = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
        const auto s = device.width() / double(view_size.x);
        const auto d = view_size / 2.0;
        return ObjectTransformation().scaled(Vec2f(s, s)).apply(t.translated(d));
      }
    }();

    scene.object_tree().root().set_transformation(transformation);
    scene.evaluate_tags();

    Painter::Options options(device);
    renderer.render(options);
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
  file_dialog.setDirectory(m_filepath);
  file_dialog.setNameFilter(tr("Images (*.png *.jpg)"));
  file_dialog.setFileMode(QFileDialog::AnyFile);
  file_dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (file_dialog.exec() == QDialog::Accepted) {
    const auto filenames = file_dialog.selectedFiles();
    assert(filenames.size() == 1);
    const auto& filename = filenames.front();

    QImage image(m_ui->ne_resolution_x->value(),
                 m_ui->ne_resolution_y->value(),
                 QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    render(m_scene, view(), image);
    if (image.save(filename)) {
      m_filepath = filename;
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
  file_dialog.setDirectory(m_filepath);
  file_dialog.setNameFilter(tr("SVG (*.svg)"));
  file_dialog.setFileMode(QFileDialog::AnyFile);
  file_dialog.setAcceptMode(QFileDialog::AcceptSave);

  if (file_dialog.exec() == QDialog::Accepted) {
    const auto filenames = file_dialog.selectedFiles();
    assert(filenames.size() == 1);
    const auto& filename = filenames.front();

    QSvgGenerator generator;
    generator.setFileName(filename);
    const auto scale = m_ui->ne_scaling->value();
    auto view_box_size = view()->property(View::SIZE_PROPERTY_KEY)->value<Vec2f>();
    view_box_size *= scale / view_box_size.x;
    generator.setViewBox(QRectF(0.0, 0.0, view_box_size.x, view_box_size.y));
    static constexpr double SVG_SCALE_FACTOR = -4.0 / 3.0;
    render(m_scene, view(), generator, SVG_SCALE_FACTOR * scale);
    m_filepath = filename;
  }
}

QString ExportDialog::filename(QString pattern, int frame_number)
{
  const int first_occurence = pattern.indexOf(frame_number_placeholder);
  if (first_occurence < 0) {
    qWarning() << "Frame number placeholder '" << frame_number_placeholder
               << "' does not occur in pattern '" << pattern << "'.";
    return pattern;
  }
  const int last_occurence = pattern.lastIndexOf(frame_number_placeholder);
  const int n = last_occurence - first_occurence + 1;
  assert(n > 0);
  if (pattern.mid(first_occurence, n) != QString(frame_number_placeholder).repeated(n)) {
    qWarning() << "Pattern '" << pattern << "' is illegal: non-contiguous sequence of '"
               << frame_number_placeholder << "'.";
  }
  static constexpr int base = 10;
  pattern.replace(first_occurence, n, QString("%1").arg(frame_number, n, base, QChar('0')));
  return pattern;
}

void ExportDialog::update_active_view()
{
  m_ui->cb_view->update_candidates();
  for (auto* view : type_casts<View*>(m_scene.object_tree().items())) {
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

void ExportDialog::restore_settings()
{
  QSettings settings;
  m_ui->cb_format->setCurrentIndex(settings.value(FORMAT_SETTINGS_KEY, 0).toInt());
}

void ExportDialog::set_default_values(Scene& scene)
{
  constexpr  int default_resolution_x = 1000;
  m_ui->ne_resolution_x->set_value(default_resolution_x);

  m_ui->cb_view->set_filter(PropertyFilter({Kind::Object}, {{Flag::IsView}}));
  m_ui->cb_view->set_null_label(tr("Viewport"));
  m_ui->cb_view->set_scene(scene);

  static constexpr double SCALING_DEFAULT = 100.0;
  static constexpr double SCALING_STEP = 0.01;
  m_ui->ne_scaling->set_value(SCALING_DEFAULT);
  m_ui->ne_scaling->set_step(SCALING_STEP);

  m_ui->le_pattern->setText(scene.animator().filename_pattern);
  m_ui->cb_overwrite->setChecked(scene.animator().overwrite_file);
  m_ui->le_pattern->setValidator(m_validator.get());

  if (!m_ui->le_pattern->hasAcceptableInput()) {
    m_ui->le_pattern->setText(tr("frame_%%%%.png"));
  }
  this->set_maximum_start(m_ui->sb_end->value());
  this->set_minimum_end(m_ui->sb_start->value());

  for (auto&& pb : {m_ui->pb_reset_end, m_ui->pb_reset_start}) {
    pb->setIcon(QIcon(":/icons/revert.png"));
    Application::instance().register_auto_invert_icon_button(*pb);
  }
}

void ExportDialog::connect_gui()
{
  connect(m_ui->pb_export, &QPushButton::clicked, this, &ExportDialog::save_as);
  QObject::connect(m_ui->cb_view, &ReferenceLineEdit::value_changed, [this]() {
    update_y_edit();
    update_preview();
  });

  const auto update_x_edit = &ExportDialog::update_x_edit;
  QObject::connect(m_ui->ne_resolution_y, &IntNumericEdit::value_changed, this, update_x_edit);

  const auto update_y_edit = &ExportDialog::update_y_edit;
  QObject::connect(m_ui->ne_resolution_x, &IntNumericEdit::value_changed, this, update_y_edit);

  const auto update_pattern_edit_background = &ExportDialog::update_pattern_edit_background;
  connect(m_ui->le_pattern, &QLineEdit::textChanged, this, update_pattern_edit_background);

  connect(m_ui->pb_reset_end, &QPushButton::clicked, this, &ExportDialog::reset_end_frame);
  connect(m_ui->pb_reset_start, &QPushButton::clicked, this, &ExportDialog::reset_start_frame);

  const auto set_minimum_end = &ExportDialog::set_minimum_end;
  connect(m_ui->sb_start, qOverload<int>(&QSpinBox::valueChanged), this, set_minimum_end);

  const auto set_maximum_start = &ExportDialog::set_maximum_start;
  connect(m_ui->sb_end, qOverload<int>(&QSpinBox::valueChanged), this, set_maximum_start);

  connect(m_ui->pb_start, &QPushButton::clicked, this, &ExportDialog::start_export_animation);
}

void ExportDialog::update_pattern_edit_background()
{
  QPalette palette = m_ui->le_pattern->palette();
  if (m_ui->le_pattern->hasAcceptableInput()) {
    palette.setColor(QPalette::Base, ui_color(*this, QPalette::Base));
    m_ui->pb_start->setEnabled(true);
  } else {
    palette.setColor(QPalette::Base, ui_color(*this, "Widget", "invalid base"));
    m_ui->pb_start->setEnabled(false);
  }
  m_ui->le_pattern->setPalette(palette);
}

void ExportDialog::reset_start_frame()
{
  const int start_frame = m_scene.animator().start();
  m_ui->sb_end->setValue(std::max(m_ui->sb_end->value(), start_frame + 1));
  m_ui->sb_start->setValue(start_frame);
}

void ExportDialog::reset_end_frame()
{
  const int end_frame = m_scene.animator().end();
  m_ui->sb_start->setValue(std::min(m_ui->sb_start->value(), end_frame - 1));
  m_ui->sb_end->setValue(end_frame);
}

void ExportDialog::set_maximum_start(int max)
{
  m_ui->sb_start->setMaximum(max - 1);
}

void ExportDialog::set_minimum_end(int min)
{
  m_ui->sb_end->setMinimum(min + 1);
}

void ExportDialog::start_export_animation()
{
  const QFileDialog::Options options{};
  const QString path = QFileDialog::getExistingDirectory(this,
                                                         tr("Export animation to ..."),
                                                         m_animation_directory,
                                                         options);
  if (!path.isEmpty()) {
    m_animation_directory = path;
    const QString pattern = m_ui->le_pattern->text();
    const bool allow_overwrite = m_ui->cb_overwrite->isChecked();
    for (int frame = m_ui->sb_start->value(); frame <= m_ui->sb_end->value(); ++frame) {
      const QString filename = path + "/" + ExportDialog::filename(pattern, frame);
      if (QFileInfo::exists(filename) && !allow_overwrite) {
        LINFO << "Did not overwrite '" << filename << "'.";
      } else {
        m_scene.animator().set_current(frame);
        QImage image(m_ui->ne_resolution_x->value(),
                     m_ui->ne_resolution_y->value(),
                     QImage::Format_ARGB32_Premultiplied);
        if (image.save(filename)) {
          LINFO << "Wrote '" << filename << "'.";
        } else {
          LWARNING << "Failed to write '" << filename << "'";
        }
      }
    }
  }
}

void ExportDialog::update_x_edit()
{
  update_edit(*m_ui->ne_resolution_y, *m_ui->ne_resolution_x, compute_aspect_ratio(view()));
}

void ExportDialog::update_y_edit()
{
  update_edit(*m_ui->ne_resolution_x, *m_ui->ne_resolution_y, 1.0 / compute_aspect_ratio(view()));
}

void ExportDialog::save_as()
{
  switch (m_ui->cb_format->currentIndex()) {
  case 0:
    save_as_raster();
    break;
  case 1:
    save_as_svg();
    break;
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
  m_ui->stackedWidget->setCurrentIndex(m_ui->cb_format->currentIndex());
  QDialog::showEvent(e);
}

void ExportDialog::hideEvent(QHideEvent* e)
{
  save_settings();
  m_scene.animator().filename_pattern = m_ui->le_pattern->text();
  m_scene.animator().overwrite_file = m_ui->cb_overwrite->isChecked();
  QDialog::hideEvent(e);
}

}  // namespace omm
