#include "managers/timeline/timeline.h"
#include "ui_timeline.h"
#include "scene/scene.h"
#include "scene/animator.h"

namespace omm
{

TimeLine::TimeLine(Scene &scene)
  : Manager(QCoreApplication::translate("any-context", "TimeLine"), scene)
  , m_ui(new Ui::TimeLine)
{
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  set_widget(std::move(widget));

  m_ui->slider->set_min(scene.animator().start());
  m_ui->slider->set_max(scene.animator().end());
  m_ui->slider->set_value(scene.animator().current());
  m_ui->sp_min->setMaximum(scene.animator().current());
  m_ui->sp_min->setValue(scene.animator().start());
  m_ui->sp_max->setValue(scene.animator().end());
  m_ui->sp_max->setMinimum(scene.animator().current());
  m_ui->sp_value->setRange(scene.animator().start(), scene.animator().end());
  m_ui->sp_value->setValue(scene.animator().current());
  LINFO << "current: " << scene.animator().current();

  connect(&scene.animator(), SIGNAL(end_changed(int)), m_ui->slider, SLOT(set_max(int)));
  connect(&scene.animator(), SIGNAL(start_changed(int)), m_ui->slider, SLOT(set_min(int)));
  connect(&scene.animator(), SIGNAL(current_changed(int)), m_ui->slider, SLOT(set_value(int)));

  connect(m_ui->slider, SIGNAL(value_changed(int)), &scene.animator(), SLOT(set_current(int)));
  connect(m_ui->sp_max, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_end(int)));
  connect(m_ui->sp_min, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_start(int)));
  connect(m_ui->sp_value, SIGNAL(valueChanged(int)), &scene.animator(), SLOT(set_current(int)));

  connect(&scene.animator(), &Animator::end_changed, [this](int start) {
    m_ui->sp_max->setValue(start);
    m_ui->sp_value->setMaximum(start);
  });
  connect(&scene.animator(), &Animator::start_changed, [this](int start) {
    m_ui->sp_min->setValue(start);
    m_ui->sp_value->setMinimum(start);
  });
  connect(&scene.animator(), &Animator::current_changed, [this](int current) {
    m_ui->sp_value->setValue(current);
    m_ui->sp_max->setMinimum(current);
    m_ui->sp_min->setMaximum(current);
  });

  connect(m_ui->pb_reset, &QPushButton::clicked,
          &scene.animator(), [this, &scene]() {
    scene.animator().set_current(scene.animator().start());
  });
  connect(m_ui->pb_play, SIGNAL(toggled(bool)), &scene.animator(), SLOT(toggle_play_pause(bool)));
  connect(&scene.animator(), SIGNAL(play_pause_toggled(bool)),
          m_ui->pb_play, SLOT(setChecked(bool)));
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

}  // namespace omm
