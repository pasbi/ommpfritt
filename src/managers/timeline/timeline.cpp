#include "managers/timeline/timeline.h"
#include "animation/animator.h"
#include "main/application.h"
#include "mainwindow/iconprovider.h"
#include "managers/timeline/slider.h"
#include "managers/timeline/timelinetitlebar.h"
#include "scene/scene.h"
#include "ui_timelinetitlebar.h"

namespace omm
{
TimeLine::TimeLine(Scene& scene)
    : Manager(QCoreApplication::translate("any-context", "TimeLine"), scene)
{
  auto title_bar = std::make_unique<TimeLineTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  auto slider = std::make_unique<Slider>(scene.animator());
  m_slider = slider.get();
  set_widget(std::move(slider));

  static constexpr auto flip = IconProvider::Orientation::FlippedHorizontally;
  m_slider->set_range(scene.animator().start(), scene.animator().end());
  m_title_bar->ui()->sp_min->setValue(scene.animator().start());
  m_title_bar->ui()->sp_max->setValue(scene.animator().end());
  m_title_bar->ui()->sp_value->setValue(scene.animator().current());
  m_title_bar->ui()->pb_jump_left->setIcon(IconProvider::pixmap("jump-end", flip));
  m_title_bar->ui()->pb_jump_right->setIcon(IconProvider::pixmap("jump-end"));
  m_title_bar->ui()->pb_step_left->setIcon(IconProvider::pixmap("next-frame", flip));
  m_title_bar->ui()->pb_step_right->setIcon(IconProvider::pixmap("next-frame"));
  m_title_bar->ui()->pb_jump_left_key->setIcon(IconProvider::pixmap("next-frame-key", flip));
  m_title_bar->ui()->pb_jump_right_key->setIcon(IconProvider::pixmap("next-frame-key"));

  connect(&scene.animator(), &Animator::end_changed, this, [this](int end) {
    m_title_bar->ui()->sp_min->setValue(std::min(end, m_title_bar->ui()->sp_min->value()));
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_title_bar->ui()->sp_max->setValue(std::max(start, m_title_bar->ui()->sp_max->value()));
  });
  connect(m_slider, &Slider::value_changed, &scene.animator(), &Animator::set_current);
  connect(m_title_bar->ui()->sp_max,
          qOverload<int>(&QSpinBox::valueChanged),
          &scene.animator(),
          &Animator::set_end);
  connect(m_title_bar->ui()->sp_min,
          qOverload<int>(&QSpinBox::valueChanged),
          &scene.animator(),
          &Animator::set_start);
  connect(m_title_bar->ui()->sp_value,
          qOverload<int>(&QSpinBox::valueChanged),
          &scene.animator(),
          &Animator::set_current);

  connect(&scene.animator(), &Animator::end_changed, this, [this](int start) {
    m_title_bar->ui()->sp_max->setValue(start);
  });
  connect(&scene.animator(), &Animator::start_changed, this, [this](int start) {
    m_title_bar->ui()->sp_min->setValue(start);
  });
  connect(&scene.animator(), &Animator::current_changed, this, [this](int current) {
    m_title_bar->ui()->sp_value->setValue(current);
  });

  connect(m_title_bar->ui()->pb_jump_left, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().set_current(scene.animator().start());
  });
  connect(m_title_bar->ui()->pb_jump_right, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().set_current(scene.animator().end());
  });
  connect(m_title_bar->ui()->pb_play_left, &QPushButton::clicked, [&scene](bool clicked) {
    scene.animator().set_play_direction(clicked ? Animator::PlayDirection::Backward
                                                : Animator::PlayDirection::Stopped);
  });
  connect(m_title_bar->ui()->pb_play_right, &QPushButton::clicked, [&scene](bool clicked) {
    scene.animator().set_play_direction(clicked ? Animator::PlayDirection::Forward
                                                : Animator::PlayDirection::Stopped);
  });
  connect(m_title_bar->ui()->pb_jump_left_key, &QPushButton::clicked, &scene.animator(), [this]() {
    jump_to_next_keyframe(Animator::PlayDirection::Backward);
  });
  connect(m_title_bar->ui()->pb_jump_right_key, &QPushButton::clicked, &scene.animator(), [this]() {
    jump_to_next_keyframe(Animator::PlayDirection::Forward);
  });
  connect(m_title_bar->ui()->pb_step_left, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().advance(Animator::PlayDirection::Backward);
  });
  connect(m_title_bar->ui()->pb_step_right, &QPushButton::clicked, &scene.animator(), [&scene]() {
    scene.animator().advance(Animator::PlayDirection::Forward);
  });
  connect(m_title_bar->ui()->cb_mode,
          qOverload<int>(&QComboBox::currentIndexChanged),
          &scene.animator(),
          [&scene](int mode) {
            scene.animator().set_play_mode(static_cast<Animator::PlayMode>(mode));
          });
  connect(&scene.animator(),
          &Animator::play_direction_changed,
          this,
          [this](Animator::PlayDirection d) { update_play_pause_button(d); });
  update_play_pause_button(scene.animator().play_direction());
  connect(&scene.animator(), &Animator::play_mode_changed, this, [this](Animator::PlayMode mode) {
    m_title_bar->ui()->cb_mode->setCurrentIndex(static_cast<int>(mode));
  });
  m_title_bar->ui()->cb_mode->setCurrentIndex(static_cast<int>(scene.animator().play_mode()));
}

void TimeLine::update_play_pause_button(Animator::PlayDirection direction)
{
  const bool forward = direction == Animator::PlayDirection::Forward;
  const bool backward = direction == Animator::PlayDirection::Backward;
  m_title_bar->ui()->pb_play_left->setChecked(backward);
  m_title_bar->ui()->pb_play_right->setChecked(forward);
  static constexpr auto flip = IconProvider::Orientation::FlippedHorizontally;
  m_title_bar->ui()->pb_play_left->setIcon(IconProvider::pixmap(backward ? "pause" : "play", flip));
  m_title_bar->ui()->pb_play_right->setIcon(IconProvider::pixmap(forward ? "pause" : "play"));
}

void TimeLine::jump_to_next_keyframe(Animator::PlayDirection direction)
{
  std::set<int> key_frames;
  for (auto&& track : m_slider->tracks()) {
    const auto kf = track->key_frames();
    key_frames.insert(kf.begin(), kf.end());
  }

  const bool forward = direction == Animator::PlayDirection::Forward;
  bool has_next_keyframe = false;
  int current_keyframe = scene().animator().current();
  int next_keyframe = forward ? std::numeric_limits<int>::max() : std::numeric_limits<int>::min();
  for (auto&& frame : key_frames) {
    const bool mv_forward = forward && frame > current_keyframe && frame < next_keyframe;
    const bool mv_backward = !forward && frame < current_keyframe && frame > next_keyframe;
    if (mv_forward || mv_backward) {
      has_next_keyframe = true;
      next_keyframe = frame;
    }
  }

  if (has_next_keyframe) {
    scene().animator().set_current(next_keyframe);
  } else {
    LWARNING << "There are no more keyframes in "
             << (direction == Animator::PlayDirection::Forward ? "forward" : "backward")
             << "-direction.";
  }
}

bool TimeLine::perform_action(const QString& name)
{
  LINFO << name;
  return false;
}

}  // namespace omm
