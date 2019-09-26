#include "managers/timeline/timeline.h"
#include "ui_timeline.h"
#include "scene/scene.h"
#include "animation/animator.h"

namespace omm
{

TimeLine::TimeLine(Scene &scene)
  : Manager(QCoreApplication::translate("any-context", "TimeLine"), scene)
  , m_ui(new Ui::TimeLine)
{
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  m_ui->slider->set_animator(scene.animator());
  set_widget(std::move(widget));

  m_ui->slider->set_min(scene.animator().start());
  m_ui->slider->set_max(scene.animator().end());
  m_ui->sp_min->setValue(scene.animator().start());
  m_ui->sp_max->setValue(scene.animator().end());
  m_ui->sp_value->setValue(scene.animator().current());
  m_ui->pb_reset->setIcon(QIcon(QPixmap::fromImage(QImage(":/icons/Jump-End.png").mirrored(true, false))));

  connect(&scene.animator(), &Animator::end_changed, this, [this](int end) {
    m_ui->sp_min->setValue(std::min(end, m_ui->sp_min->value()));
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_ui->sp_max->setValue(std::max(start, m_ui->sp_max->value()));
  });
  connect(m_ui->slider, SIGNAL(value_changed(int)), &scene.animator(), SLOT(set_current(int)));
  connect(m_ui->sp_max, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_end(int)));
  connect(m_ui->sp_min, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_start(int)));
  connect(m_ui->sp_value, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_current(int)));

  connect(&scene.animator(), &Animator::end_changed, this, [this](int start) {
    m_ui->sp_max->setValue(start);
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_ui->sp_min->setValue(start);
  });
  connect(&scene.animator(), &Animator::current_changed, this, [this](int current) {
    m_ui->sp_value->setValue(current);
  });

  connect(m_ui->pb_reset, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().set_current(scene.animator().start());
  });
  connect(m_ui->pb_play, SIGNAL(toggled(bool)), &scene.animator(), SLOT(toggle_play_pause(bool)));
  connect(&scene.animator(), SIGNAL(play_pause_toggled(bool)),
          this, SLOT(update_play_pause_button(bool)));
  update_play_pause_button(scene.animator().is_playing());
}

std::vector<CommandInterface::ActionInfo<TimeLine>> TimeLine::action_infos()
{
  using AI = ActionInfo<TimeLine>;
  return {
  };
}

void TimeLine::UiTimeLineDeleter::operator()(Ui::TimeLine *ui)
{
  delete ui;
}

void TimeLine::update_play_pause_button(bool play)
{
  m_ui->pb_play->setChecked(play);
  m_ui->pb_play->setIcon(QIcon(QString(":/icons/%1.png").arg(play ? "Pause" : "Play")));
}
}  // namespace omm
