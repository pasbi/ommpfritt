#include "mainwindow/exportdialog.h"
#include "stringinterpolation.h"
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
#include <QAbstractListModel>

namespace
{

static constexpr auto ALLOW_OVERWRITE_KEY = "allow_overwrite";

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

double compute_aspect_ratio(const omm::View* view)
{
  if (view == nullptr) {
    return double(viewport().width()) / double(viewport().height());
  } else {
    const omm::Vec2f s = view->property(omm::View::SIZE_PROPERTY_KEY)->value<omm::Vec2f>();
    return s.x / s.y;
  }
}

void update_edit(omm::IntNumericEdit& primary, omm::IntNumericEdit& secondary, double aspect_ratio)
{
  const int p_value = primary.value();
  const int s_value = static_cast<int>(p_value * aspect_ratio);
  QSignalBlocker blocker(secondary);
  secondary.set_value(s_value);
}

QString interpolate_filename(const QString& pattern,
                             const QString& scene_path,
                             const QString& scene_name,
                             int frame)
{
  return omm::StringInterpolation(pattern, {
      {"path", scene_path},
      {"name", scene_name},
      {"frame", frame}
  });
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
      interpolate_filename(input, "", "", 0);
      StringInterpolation(input, m_values);
    } catch (const StringInterpolation::InvalidFormatException& e) {
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

  void set_variable_values(const StringInterpolation::map_type& values)
  {
    m_values = values;
  }

private:
  std::vector<QString> m_allowed_endings;
  StringInterpolation::map_type m_values;
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
    : QDialog(parent), m_scene(scene)
    , m_ui(std::make_unique<::Ui::ExportDialog>())
    , m_validator(std::make_unique<FilenamePatternValidator>())
    , m_raster_format_list_model(new MapListModel{{{tr("PNG"), ".png"}, {tr("JPEG"), ".jpg"}}})
    , m_svg_format_list_model(new MapListModel{{{tr("SVG"), ".svg"}}})
    , m_variable_list_model(new MapListModel{{{tr("Scene Name"), "{name}"},
                                              {tr("Scene Path"), "{path}"},
                                              {tr("Frame Number"), "{frame:04}"}}})
{
  m_ui->setupUi(this);
  set_default_values();
  connect_gui();
  update_y_edit();
  update_preview();
  update_active_view();
  update_pattern_edit_background();
  m_ui->splitter->setSizes({width(), 1});
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
  return type_cast<const View*>(kind_cast<Object*>(m_ui->cb_view->value()));
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

bool ExportDialog::save_as_raster(const QString& filename)
{
  QImage image(m_ui->ne_resolution_x->value(),
               m_ui->ne_resolution_y->value(),
               QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  render(m_scene, view(), image);
  LINFO << "Wrote file '" << filename << "'.";
  return image.save(filename);
}

bool ExportDialog::save_as_svg(const QString& filename)
{
  QSvgGenerator generator;
  generator.setFileName(filename);
  const auto scale = m_ui->ne_scaling->value();
  auto view_box_size = view()->property(View::SIZE_PROPERTY_KEY)->value<Vec2f>();
  view_box_size *= scale / view_box_size.x;
  generator.setViewBox(QRectF(0.0, 0.0, view_box_size.x, view_box_size.y));
  static constexpr double SVG_SCALE_FACTOR = -4.0 / 3.0;
  render(m_scene, view(), generator, SVG_SCALE_FACTOR * scale);
  LINFO << "Wrote file '" << filename << "'.";
  return true;
}

QString ExportDialog::filename(int frame) const
{
  const QFileInfo scene_fi(m_scene.filename());
  const auto scene_path = scene_fi.dir().path();
  const auto scene_name = scene_fi.fileName();

  QString pattern = m_ui->le_pattern->path();

  if (!QFileInfo(pattern).isAbsolute()) {
    pattern = scene_path + "/" + pattern;
  }
  if (!QFileInfo(pattern).isAbsolute()) {
    pattern = QApplication::applicationDirPath() + "/" + pattern;
  }

  return ::interpolate_filename(pattern, scene_path, scene_name, frame);
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
  settings.setValue(ALLOW_OVERWRITE_KEY, m_ui->cb_overwrite->isChecked());

  auto& eo = m_scene.export_options;
  eo.pattern = m_ui->le_pattern->path();
  eo.start_frame = m_ui->sb_start->value();
  eo.end_frame = m_ui->sb_end->value();
  eo.animated = m_ui->cb_animation->isChecked();
  eo.view = type_cast<View*>(m_ui->cb_view->value());
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

  const auto& eo = m_scene.export_options;
  m_ui->le_pattern->set_path(eo.pattern);
  m_ui->cb_animation->setChecked(eo.animated);
  m_ui->cb_view->set_value(eo.view);
  set_animation_range(eo.start_frame, eo.end_frame);
}

void ExportDialog::set_default_values()
{
  static constexpr int default_resolution_x = 1000;
  static constexpr double SCALING_DEFAULT = 100.0;
  static constexpr double SCALING_STEP = 0.01;

  m_ui->ne_resolution_x->set_value(default_resolution_x);

  m_ui->cb_view->set_filter(PropertyFilter({Kind::Object}, {{Flag::IsView}}));
  m_ui->cb_view->set_null_label(tr("Viewport"));
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
    pb->setIcon(QIcon(":/icons/revert.png"));
    Application::instance().register_auto_invert_icon_button(*pb);
  }

  m_ui->cb_variable->setModel(m_variable_list_model.get());
  m_ui->cb_ending->setModel(m_raster_format_list_model.get());
  restore_settings();
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
                                          static_cast<const MapListModel&>(*cb.model()).codes.at(i),
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
  connect(&m_ui->le_pattern->line_edit(), &QLineEdit::textChanged, this, [this]() {
    update_pattern_edit_background();
    update_ending_cb();
  });
}

void ExportDialog::update_pattern_edit_background()
{
  QPalette palette = m_ui->le_pattern->palette();
  if (m_ui->le_pattern->line_edit().hasAcceptableInput()) {
    palette.setColor(QPalette::Base, ui_color(*this, QPalette::Base));
    m_ui->pb_start->setEnabled(true);
    m_ui->pb_start->setToolTip("");
  } else {
    palette.setColor(QPalette::Base, ui_color(*this, "Widget", "invalid base"));
    m_ui->pb_start->setEnabled(false);
    m_ui->pb_start->setToolTip(tr("Invalid pattern."));
  }

  m_ui->le_pattern->setPalette(palette);
}

void ExportDialog::update_ending_cb()
{
  const QString ending = "." + QFileInfo(m_ui->le_pattern->path()).suffix().toLower();
  const auto endings = static_cast<const MapListModel&>(*m_ui->cb_ending->model()).codes;
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

int ExportDialog::render(int frame, bool allow_overwrite)
{
  const auto mb = [this](const QMessageBox::Icon icon, const QString& title, const QString& msg) {
    const auto buttons = QMessageBox::Cancel | QMessageBox::Ignore | QMessageBox::Retry;
    QMessageBox mb{icon, title, msg, buttons, this};
    mb.setDefaultButton(QMessageBox::Cancel);
    return mb.exec();
  };

  try {
    const QString filename = this->filename(frame);
    const QFileInfo fi(filename);
    const auto dir = fi.dir();
    if (!dir.exists()) {
      if (!dir.mkpath(dir.path())) {
        LERROR << "Failed to create directory '" << dir.path() << "'.";
        return mb(QMessageBox::Critical,
                  tr("Failed to create directory"),
                  tr("Failed to create directory '%1'.").arg(dir.path()));
      } else {
        LINFO << "Created directory '" << dir.path() << "'.";
      }
    }

    if (fi.exists() && !allow_overwrite) {
      LINFO << "Did not overwrite existing '" << filename << "'.";
      return mb(QMessageBox::Information,
                tr("Failed to overwrite"),
                tr("No permission to overwrite existing file '%1'.").arg(filename));
      return QMessageBox::Ignore;
    } else if (QFileInfo(fi.dir().path()).isWritable()) {
      const auto save_as = [this, filename]() {
        switch (m_ui->cb_format->currentIndex()) {
        case 0:
          return save_as_raster(filename);
        case 1:
          return save_as_svg(filename);
        default:
          Q_UNREACHABLE();
        }
      };
      if (save_as()) {
        return QMessageBox::Ok;
      } else {
        LERROR << "Exporting frame " << frame << " failed.";
        return mb(QMessageBox::Critical, tr("Error"), tr("Unexpected error."));
      }
    } else {
      LERROR << "Failed to write '" << filename << "'.";
      return mb(QMessageBox::Critical,
                tr("Invalid filename"),
                tr("The filename '%1' cannot be written.").arg(filename));
    }
  } catch (StringInterpolation::InvalidFormatException& e) {
    return mb(QMessageBox::Critical, tr("Invalid Pattern"), tr("The filename pattern is invalid."));
  }
}

void ExportDialog::start_export()
{
  const bool allow_overwrite = m_ui->cb_overwrite->isChecked();
  const bool animation = m_ui->cb_animation->isChecked();
  const int start_frame = m_ui->sb_start->value();
  const int end_frame = animation ? m_ui->sb_end->value() : start_frame;

  bool cancel = false;
  bool retry = true;
  for (int frame = start_frame; !cancel && frame <= end_frame; ++frame) {
    m_scene.animator().set_current(frame);

    do {
      const auto status = render(frame, allow_overwrite);
      retry = !!(status & QMessageBox::Retry);
      cancel = !!(status & QMessageBox::Cancel);
    } while (retry);
  }
  if (!cancel) {
    QMessageBox::information(this, tr("Done."), tr("Exporting done."));
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
  QDialog::hideEvent(e);
}

}  // namespace omm
