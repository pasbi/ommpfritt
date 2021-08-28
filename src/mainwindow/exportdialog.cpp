#include "mainwindow/exportdialog.h"
#include "animation/animator.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include "mainwindow/exporter.h"
#include "mainwindow/exportoptions.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/viewport/viewport.h"
#include "objects/view.h"
#include "preferences/uicolors.h"
#include "scene/scene.h"
#include "ui_exportdialog.h"
#include "widgets/numericedit.h"
#include "widgets/referencelineedit.h"
#include "syncpalettedecorator.h"
#include <fmt/format.h>
#include <QAbstractListModel>
#include <QFileDialog>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

namespace
{
constexpr auto ALLOW_OVERWRITE_KEY = "allow_overwrite";

template<std::size_t i, typename Ts> std::vector<QString> get(Ts&& vs)
{
  std::vector<QString> r;
  r.reserve(vs.size());
  static const auto get_i = [](auto&& p) { return std::get<i>(p); };
  std::transform(vs.begin(), vs.end(), std::back_inserter(r), get_i);
  return r;
}

omm::Viewport& viewport()
{
  return omm::Application::instance().main_window()->viewport();
}

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
class FilenamePatternValidator : public QValidator
{
public:
  QValidator::State validate(QString& input, int&) const override
  {
    try {
      Exporter::interpolate_filename(input, "", "", 0);
    } catch (const fmt::format_error& e) {
      return QValidator::Intermediate;
    }

    for (const QString& allowed_ending : m_allowed_endings) {
      if (input.endsWith(allowed_ending, Qt::CaseInsensitive)) {
        return QValidator::Acceptable;
      }
    }
    return QValidator::Intermediate;
  }

  void set_allowed_endings(const std::vector<QString>& endings)
  {
    m_allowed_endings = endings;
    Q_EMIT changed();
  }

private:
  std::vector<QString> m_allowed_endings;
};

ExportDialog::MapListModel::MapListModel(const map_type& values)
    : labels(::get<0>(values)), codes(::get<1>(values))
{
}

QVariant ExportDialog::MapListModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole) {
    return labels.at(index.row());
  } else {
    return QVariant();
  }
}

int ExportDialog::MapListModel::rowCount([[maybe_unused]] const QModelIndex& index) const
{
  assert(!index.isValid());
  assert(labels.size() == codes.size());
  return labels.size();
}

ExportDialog::ExportDialog(Scene& scene, QWidget* parent)
    : QDialog(parent), m_scene(scene), m_ui(std::make_unique<::Ui::ExportDialog>()),
      m_validator(std::make_unique<FilenamePatternValidator>()),
      m_raster_format_list_model(new MapListModel{{{tr("PNG"), ".png"}, {tr("JPEG"), ".jpg"}}}),
      m_svg_format_list_model(new MapListModel{{{tr("SVG"), ".svg"}}}),
      m_variable_list_model(new MapListModel{{{tr("Scene Name"), "{name}"},
                                              {tr("Scene Path"), "{path}"},
                                              {tr("Frame Number"), "{frame:04}"}}}),
      m_exporter(new Exporter(scene, &viewport(), &m_exporter_thread))
{
  m_ui->setupUi(this);
  set_default_values();
  connect_gui();
  update_y_edit();
  update_pattern_edit_background();
  m_ui->splitter->setSizes({width(), 1});
}

ExportDialog::~ExportDialog()
{
  m_exporter->cancel = true;
  m_exporter_thread.quit();
  static constexpr auto ONE_SECOND = 1000;
  if (!m_exporter_thread.wait(ONE_SECOND)) {
    LWARNING << "Failed to quit exporter thread.";
    m_exporter_thread.terminate();
  }
}

void ExportDialog::update_preview()
{
  int width = m_ui->lb_preview->width();
  int height = m_ui->lb_preview->height();

  const double ar = compute_aspect_ratio();
  if (double(width) / double(height) > ar) {
    width = static_cast<int>(height * ar);
  } else {
    height = static_cast<int>(width / ar);
  }

  QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
  {
    QPainter painter(&image);
    UiColors::draw_background(painter, image.rect());
  }
  m_exporter->export_options = export_options();
  m_exporter->render(image, 1.0);
  m_ui->lb_preview->setPixmap(QPixmap::fromImage(image));
}

ExportOptions ExportDialog::export_options() const
{
  ExportOptions eo;
  eo.pattern = m_ui->le_pattern->path();
  eo.start_frame = m_ui->sb_start->value();
  eo.end_frame = m_ui->sb_end->value();
  eo.animated = m_ui->cb_animation->isChecked();
  eo.view = type_cast<View*>(m_ui->cb_view->value());
  eo.x_resolution = m_ui->ne_resolution_x->value();
  eo.format = static_cast<ExportOptions::Format>(m_ui->cb_format->currentIndex());
  return eo;
}

void ExportDialog::save_settings()
{
  QSettings settings;
  settings.setValue(ALLOW_OVERWRITE_KEY, m_ui->cb_overwrite->isChecked());
  m_scene.set_export_options(export_options());
}

void ExportDialog::set_animation_range(int start, int end)
{
  m_ui->sb_start->setMaximum(end - 1);
  m_ui->sb_end->setMinimum(start + 1);

  m_ui->sb_end->setValue(end);
  m_ui->sb_start->setValue(start);
}

void ExportDialog::update_frame_range_limits()
{
  set_animation_range(m_ui->sb_start->value(), m_ui->sb_end->value());
}

void ExportDialog::restore_settings()
{
  QSettings settings;
  m_ui->cb_overwrite->setChecked(settings.value(ALLOW_OVERWRITE_KEY, false).toBool());

  const auto& eo = m_scene.export_options();
  update_exporter();
  m_ui->le_pattern->set_path(eo.pattern);
  m_ui->cb_animation->setChecked(eo.animated);
  m_ui->cb_view->set_value(eo.view);
  set_animation_range(eo.start_frame, eo.end_frame);
  m_ui->cb_format->setCurrentIndex(static_cast<int>(eo.format));
}

void ExportDialog::update_exporter()
{
  m_exporter->export_options = export_options();
  update_view_null_label(m_exporter->view());
}

void ExportDialog::set_default_values()
{
  static constexpr int default_resolution_x = 1000;
  static constexpr double SCALING_DEFAULT = 100.0;
  static constexpr double SCALING_STEP = 0.01;

  m_ui->ne_resolution_x->set_range(0, std::numeric_limits<int>::max());
  m_ui->ne_resolution_x->set_value(default_resolution_x);

  m_ui->cb_view->set_filter(PropertyFilter({Kind::Object}, {{Flag::IsView}}));
  update_view_null_label(nullptr);
  m_ui->cb_view->set_scene(m_scene);

  m_ui->ne_scaling->set_value(SCALING_DEFAULT);
  m_ui->ne_scaling->set_step(SCALING_STEP);

  m_ui->le_pattern->line_edit().setValidator(m_validator.get());

  const auto path_variable_tooltip = tr(R"(The following placeholders may be used:
{name} -> filename of the scene without path and extension.
{frame} -> the frame number
{path} -> path of the scene (non-absolute paths are relative to the scene location)
For numeric values, placeholder and length options may be specified after a colon.
{frame:04} will prepend zeros until the string has length four.
)");
  m_ui->le_pattern->setToolTip(path_variable_tooltip);

  for (auto&& pb : {m_ui->pb_reset_end, m_ui->pb_reset_start}) {
    pb->setIcon(IconProvider::pixmap("revert"));
    SyncPaletteButtonDecorator::decorate(*pb);
  }

  m_ui->cb_variable->setModel(m_variable_list_model.get());
  m_ui->cb_ending->setModel(m_raster_format_list_model.get());
  restore_settings();

  if (m_ui->le_pattern->path().isEmpty()) {
    m_ui->le_pattern->set_path(tr("{name}_{frame:04}.png"));
  }

  update_enabledness(false);
  m_ui->te_status->hide();
}

void ExportDialog::connect_gui()
{
  QObject::connect(m_ui->cb_view, &ReferenceLineEdit::value_changed, [this]() {
    update_y_edit();
    update_preview();
  });

  const auto update_x_edit = &ExportDialog::update_x_edit;
  QObject::connect(m_ui->ne_resolution_y, &IntNumericEdit::value_changed, this, update_x_edit);

  const auto update_y_edit = &ExportDialog::update_y_edit;
  QObject::connect(m_ui->ne_resolution_x, &IntNumericEdit::value_changed, this, update_y_edit);

  connect(m_ui->pb_reset_end, &QPushButton::clicked, this, &ExportDialog::reset_end_frame);
  connect(m_ui->pb_reset_start, &QPushButton::clicked, this, &ExportDialog::reset_start_frame);

  for (auto&& sb : {m_ui->sb_start, m_ui->sb_end}) {
    auto&& update_frame_range_limits = &ExportDialog::update_frame_range_limits;
    connect(sb, qOverload<int>(&QSpinBox::valueChanged), this, update_frame_range_limits);
  }

  connect(m_ui->pb_start, &QPushButton::clicked, this, &ExportDialog::start_export);
  connect(m_ui->splitter, &QSplitter::splitterMoved, this, &ExportDialog::update_preview);

  connect(m_ui->cb_animation, &QCheckBox::toggled, m_ui->sb_end, &QSpinBox::setEnabled);
  connect(m_ui->cb_animation, &QCheckBox::toggled, m_ui->pb_reset_end, &QPushButton::setEnabled);
  m_ui->sb_end->setEnabled(m_ui->cb_animation->isChecked());
  m_ui->pb_reset_end->setEnabled(m_ui->cb_animation->isChecked());

  const auto connect_conv_cb = [this](QComboBox& cb, auto&& f) {
    cb.setCurrentIndex(-1);
    connect(&cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [&, this](int i) {
      if (i >= 0) {
        cb.setCurrentIndex(-1);
        auto&& [new_text, cursor_pos] = f(m_ui->le_pattern->path(),
                                          dynamic_cast<const MapListModel&>(*cb.model()).codes.at(i),
                                          m_ui->le_pattern->line_edit().cursorPosition());
        m_ui->le_pattern->set_path(new_text);
        m_ui->le_pattern->line_edit().setCursorPosition(cursor_pos);
      }
    });
  };

  connect_conv_cb(*m_ui->cb_variable, [](QString path, const QString& code, int cursor_pos) {
    path.insert(cursor_pos, code);
    return std::pair{path, cursor_pos + code.length()};
  });

  connect_conv_cb(*m_ui->cb_ending, [](QString path, const QString& code, int cursor_pos) {
    int pos = path.lastIndexOf(".");
    if (pos < 0) {
      path += code;
    } else {
      path.replace(pos, path.size(), code);
    }
    return std::pair{path, cursor_pos};
  });

  const auto update_format = [this](int i) {
    auto& current_format_model = i == 0 ? *m_raster_format_list_model : *m_svg_format_list_model;
    m_ui->cb_ending->setModel(&current_format_model);
    m_validator->set_allowed_endings(current_format_model.codes);
    Q_EMIT m_validator->changed();
    update_ending_cb();
  };

  connect(m_ui->cb_format, qOverload<int>(&QComboBox::currentIndexChanged), this, update_format);
  update_format(m_ui->cb_format->currentIndex());

  connect(m_validator.get(), &QValidator::changed, this, [this]() {
    update_pattern_edit_background();
  });
  connect(m_ui->le_pattern, &FilePathEdit::path_changed, this, [this]() {
    update_pattern_edit_background();
    update_ending_cb();
  });
  connect(&*m_exporter, &Exporter::post_status, m_ui->te_status, &QPlainTextEdit::appendPlainText);
  connect(&*m_exporter, &Exporter::progress_changed, this, [this](int current, int total) {
    m_ui->pb_progress->setMaximum(total);
    m_ui->pb_progress->setValue(current);
  });
  connect(m_ui->pb_cancel, &QPushButton::clicked, this, [this]() { m_exporter->cancel = true; });
  connect(&*m_exporter, &Exporter::auto_view_changed, this, &ExportDialog::update_view_null_label);

  connect(&*m_exporter, &Exporter::finished, this, [&]() { update_enabledness(false); });
  connect(&*m_exporter, &Exporter::started, this, [&]() { update_enabledness(true); });
  connect(m_ui->cb_view, &ReferenceLineEdit::value_changed, this, &ExportDialog::update_exporter);
}

void ExportDialog::update_enabledness(bool job_running)
{
  m_ui->pb_cancel->setEnabled(job_running);
  m_ui->pb_progress->setVisible(job_running);
  m_ui->pb_cancel->setVisible(job_running);
  m_ui->pb_start->setEnabled(!job_running && m_options_are_plausible);
  m_ui->pb_start->setVisible(!job_running);

  const std::set<QWidget*> option_widgets{
      m_ui->cb_format,
      m_ui->cb_view,
      m_ui->ne_resolution_x,
      m_ui->ne_resolution_y,
      m_ui->cb_overwrite,
      m_ui->cb_animation,
      m_ui->sb_start,
      m_ui->sb_end,
      m_ui->le_pattern,
      m_ui->pb_reset_end,
      m_ui->pb_reset_start,
      m_ui->cb_variable,
      m_ui->cb_ending,
  };
  for (auto&& w : option_widgets) {
    w->setEnabled(!job_running);
  }

  if (job_running) {
    m_ui->te_status->show();
  }
}

void ExportDialog::update_pattern_edit_background()
{
  QPalette palette = m_ui->le_pattern->palette();
  if (m_ui->le_pattern->line_edit().hasAcceptableInput()) {
    palette.setColor(QPalette::Base, ui_color(*this, QPalette::Base));
    m_options_are_plausible = true;
    m_ui->pb_start->setToolTip("");
  } else {
    palette.setColor(QPalette::Base, ui_color(*this, "Widget", "invalid base"));
    m_options_are_plausible = false;
    m_ui->pb_start->setToolTip(tr("Invalid pattern."));
  }
  update_enabledness(m_exporter_thread.isRunning());

  m_ui->le_pattern->setPalette(palette);
}

void ExportDialog::update_ending_cb()
{
  const QString ending = "." + QFileInfo(m_ui->le_pattern->path()).suffix().toLower();
  const auto endings = dynamic_cast<const MapListModel&>(*m_ui->cb_ending->model()).codes;
  const auto it = std::find(endings.begin(), endings.end(), ending);
  QSignalBlocker blocker(m_ui->cb_ending);
  if (it == endings.end()) {
    m_ui->cb_ending->setCurrentIndex(-1);
  } else {
    m_ui->cb_ending->setCurrentIndex(std::distance(endings.begin(), it));
  }
}

void ExportDialog::reset_start_frame()
{
  const int start_frame = m_scene.animator().start();
  const int end_frame = std::max(m_ui->sb_end->value(), start_frame + 1);
  set_animation_range(start_frame, end_frame);
}

void ExportDialog::reset_end_frame()
{
  const int end_frame = m_scene.animator().end();
  const int start_frame = std::min(m_ui->sb_start->value(), end_frame - 1);
  set_animation_range(start_frame, end_frame);
}

void ExportDialog::start_export()
{
  m_exporter->allow_overwrite = m_ui->cb_overwrite->isChecked();
  m_exporter->y_resolution = m_ui->ne_resolution_y->value();
  m_exporter->export_options = export_options();

  m_ui->te_status->clear();

  m_exporter_thread.start();
}

void ExportDialog::update_x_edit()
{
  update_edit(*m_ui->ne_resolution_y, *m_ui->ne_resolution_x, compute_aspect_ratio());
}

void ExportDialog::update_y_edit()
{
  update_edit(*m_ui->ne_resolution_x, *m_ui->ne_resolution_y, 1.0 / compute_aspect_ratio());
}

void ExportDialog::resizeEvent(QResizeEvent* e)
{
  update_preview();
  QDialog::resizeEvent(e);
}

void ExportDialog::showEvent(QShowEvent* e)
{
  update_preview();
  m_ui->cb_view->update_candidates();
  m_ui->stackedWidget->setCurrentIndex(m_ui->cb_format->currentIndex());
  QDialog::showEvent(e);
}

void ExportDialog::hideEvent(QHideEvent* e)
{
  save_settings();
  QDialog::hideEvent(e);
}

void ExportDialog::closeEvent(QCloseEvent* e)
{
  if (m_exporter_thread.isRunning()) {
    const auto msg = tr("A rendering job is currently running. Do you want to cancel it?");
    const auto answer = QMessageBox::question(this,
                                              QApplication::applicationName(),
                                              msg,
                                              QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::No) {
      e->ignore();
      return;
    }
  }
  QDialog::closeEvent(e);
}

void ExportDialog::update_view_null_label(const View* view)
{
  const QString label = view == nullptr ? tr("Viewport") : view->name();
  m_ui->cb_view->set_null_label(tr("Auto (%1)").arg(label));
}

double ExportDialog::compute_aspect_ratio() const
{
  const View* view = m_exporter->view();
  if (view == nullptr) {
    return double(viewport().width()) / double(viewport().height());
  } else {
    const omm::Vec2f s = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
    return s.x / s.y;
  }
}

}  // namespace omm
