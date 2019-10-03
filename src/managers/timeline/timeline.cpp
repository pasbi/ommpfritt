#include "managers/timeline/timeline.h"
#include "scene/scene.h"
#include "animation/animator.h"
#include "managers/timeline/timelinetitlebar.h"
#include "managers/timeline/slider.h"
#include "ui_timelinetitlebar.h"
#include "mainwindow/application.h"

namespace omm
{

TimeLine::TimeLine(Scene &scene)
  : Manager(QCoreApplication::translate("any-context", "TimeLine"), scene)
{
  setObjectName(TYPE);

  widget()->layout()->setContentsMargins(9, 0, 9, 9);

  auto header = std::make_unique<TimeLineTitleBar>(*this);
  m_header = header.get();
  setTitleBarWidget(header.release());

  auto slider = std::make_unique<Slider>(scene.animator());
  m_slider = slider.get();
  set_widget(std::move(slider));

  m_slider->set_range(scene.animator().start(), scene.animator().end());
  m_header->ui()->sp_min->setValue(scene.animator().start());
  m_header->ui()->sp_max->setValue(scene.animator().end());
  m_header->ui()->sp_value->setValue(scene.animator().current());
  m_header->ui()->pb_reset->setIcon(QIcon(QPixmap::fromImage(QImage(":/icons/Jump-End.png").mirrored(true, false))));

  connect(&scene.animator(), &Animator::end_changed, this, [this](int end) {
    m_header->ui()->sp_min->setValue(std::min(end, m_header->ui()->sp_min->value()));
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_header->ui()->sp_max->setValue(std::max(start, m_header->ui()->sp_max->value()));
  });
  connect(m_slider, SIGNAL(value_changed(int)), &scene.animator(), SLOT(set_current(int)));
  connect(m_header->ui()->sp_max, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_end(int)));
  connect(m_header->ui()->sp_min, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_start(int)));
  connect(m_header->ui()->sp_value, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_current(int)));

  connect(&scene.animator(), &Animator::end_changed, this, [this](int start) {
    m_header->ui()->sp_max->setValue(start);
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_header->ui()->sp_min->setValue(start);
  });
  connect(&scene.animator(), &Animator::current_changed, this, [this](int current) {
    m_header->ui()->sp_value->setValue(current);
  });

  connect(m_header->ui()->pb_reset, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().set_current(scene.animator().start());
  });
  connect(m_header->ui()->pb_play, SIGNAL(toggled(bool)), &scene.animator(), SLOT(toggle_play_pause(bool)));
  connect(&scene.animator(), SIGNAL(play_pause_toggled(bool)),
          this, SLOT(update_play_pause_button(bool)));
  update_play_pause_button(scene.animator().is_playing());
}

void TimeLine::update_play_pause_button(bool play)
{
  m_header->ui()->pb_play->setChecked(play);
  m_header->ui()->pb_play->setIcon(QIcon(QString(":/icons/%1.png").arg(play ? "Pause" : "Play")));
}

bool TimeLine::perform_action(const std::string& name)
{
  LINFO << name;
  return false;
}

}  // namespace omm
