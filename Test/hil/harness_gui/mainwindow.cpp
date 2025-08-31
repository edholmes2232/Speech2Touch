#include "mainwindow.h"

#include "touch_event_parser.h"
#include "touch_targets.h"
#include "ui_Franke-A600.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEasingCurve>
#include <QEvent>
#include <QList>
#include <QMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRect>
#include <QString>
#include <QtGlobal>
#include <algorithm>
#include <cstdlib>
#include <future>
#include <memory>

void MainWindow::setupButtons()
{
  for (size_t i = 0; i < TARGET_COUNT; ++i)
  {
    TOUCH_TARGET_T target = touch_targets[i];
    qDebug() << "Setting up button for target:" << target.name;
  }

  // Get all QPushButton children from StackedWidget
  QList<QPushButton *> buttons = _ui->stackedWidget->findChildren<QPushButton *>();

  // Sort buttons by name
  std::sort(
      buttons.begin(), buttons.end(), [](QPushButton *a, QPushButton *b) { return a->objectName() < b->objectName(); });

  // Set button names, starting at 0
  // Assumes first actual target starts at index 0
  for (int i = 0; i < buttons.size(); ++i)
  {
    if (i < TARGET_COUNT)
    {
      buttons[i]->setText(touch_targets[i].name);
      connect(buttons[i], &QPushButton::released, this, [this, i]() { onButtonReleased(static_cast<TARGET_T>(i)); });
    }
    else
    {
      buttons[i]->setText(QString("Extra Button %1").arg(i - TARGET_COUNT));
      connect(buttons[i], &QPushButton::released, this, [this, i]() { onButtonReleased(static_cast<TARGET_T>(i)); });
    }
  }

  // print some sizes for debugging
  qDebug() << "Each button is " << buttons[0]->width() << "x" << buttons[0]->height();
  qDebug() << "StackedWidget size is " << _ui->stackedWidget->width() << "x" << _ui->stackedWidget->height();
}

MainWindow::MainWindow(const char *device_path, QWidget *parent)
    : QMainWindow(parent)
    , _ui(std::make_unique<Ui::MainWindow>())
    , _touch_event_parser(nullptr)
    , _was_touched(false)
{
  _ui->setupUi(this);
  setupButtons();

  _touch_event_parser = std::make_unique<TouchEventParser>(device_path,
                                                           [this](float percent_x, float percent_y, bool is_touched) {
                                                             this->processTouchEvent(percent_x, percent_y, is_touched);
                                                           });

  _ui->stackedWidget->setCurrentIndex(0);
  updateWindowTitle();

  connect(_ui->arrow_right_button, &QPushButton::clicked, this, &MainWindow::nextPage);
  connect(_ui->arrow_left_button, &QPushButton::clicked, this, &MainWindow::previousPage);

  qDebug() << "MainWindow initialized";
}

MainWindow::~MainWindow()
{
  // unique_ptr handles cleanup
  qDebug() << "MainWindow destroyed";
}

void MainWindow::processTouchEvent(float percent_x, float percent_y, bool is_touched)
{
  QPointF local_pos(percent_x * this->width(), percent_y * this->height());
  qDebug() << "Mapped to window coordinates:" << local_pos;

  QEvent::Type event_type;
  Qt::MouseButton button = Qt::LeftButton;
  Qt::MouseButtons buttons;

  if (!_was_touched && is_touched)
  {
    event_type = QEvent::MouseButtonPress;
    buttons = Qt::LeftButton;
    qDebug() << "Touch event: MouseButtonPress at" << local_pos;
  }
  else if (_was_touched && !is_touched)
  {
    event_type = QEvent::MouseButtonRelease;
    buttons = Qt::NoButton;
    qDebug() << "Touch event: MouseButtonRelease at" << local_pos;
  }
  else if (is_touched)
  {
    event_type = QEvent::MouseMove;
    buttons = Qt::LeftButton;
    qDebug() << "Touch event: MouseMove at" << local_pos;
  }
  else
  {
    qDebug() << "No touch event to process.";
    _was_touched = is_touched;
    return;
  }

  QWidget *child_widget = this->childAt(local_pos.toPoint());
  if (child_widget)
  {
    qDebug() << "Touch event target widget:" << child_widget->objectName();
  }
  else
  {
    qDebug() << "No widget found at touch position, using MainWindow.";
    child_widget = this;
  }

  QPoint widget_local_position = child_widget->mapFrom(this, local_pos.toPoint());
  QPoint global_position = child_widget->mapToGlobal(widget_local_position);

  QMouseEvent *mouse_event =
      new QMouseEvent(event_type, widget_local_position, global_position, button, buttons, Qt::NoModifier);
  QCoreApplication::postEvent(child_widget, mouse_event);

  _was_touched = is_touched;
}

// Helper to update window title
void MainWindow::updateWindowTitle()
{
  int current = _ui->stackedWidget->currentIndex() + 1;
  int total = _ui->stackedWidget->count();
  setWindowTitle(QString("Franke A600 - Page %1 of %2").arg(current).arg(total));
}

void MainWindow::nextPage()
{
  int count = _ui->stackedWidget->count();
  if (count <= 1)
    return;
  int current_index = _page_queue.isEmpty() ? _ui->stackedWidget->currentIndex() : _page_queue.last();
  if (current_index >= count - 1)
    return;
  int next_index = current_index + 1;
  _page_queue.append(next_index);
  if (_page_queue.size() == 1 && !_current_animation_group)
    switchPages(_page_queue.first());
}

void MainWindow::previousPage()
{
  int count = _ui->stackedWidget->count();
  if (count <= 1)
    return;
  int current_index = _page_queue.isEmpty() ? _ui->stackedWidget->currentIndex() : _page_queue.last();
  if (current_index <= 0)
    return;
  int prev_index = current_index - 1;
  _page_queue.append(prev_index);
  if (_page_queue.size() == 1 && !_current_animation_group)
    switchPages(_page_queue.first());
}

void MainWindow::switchPages(int index)
{
  QRect area = _ui->stackedWidget->geometry();
  int width = area.width();
  int count = _ui->stackedWidget->count();

  int current_index = _ui->stackedWidget->currentIndex();
  QWidget *current_widget = _ui->stackedWidget->widget(current_index);
  QRect current_geom = current_widget->geometry();

  static double last_progress = 0.0;
  static int last_direction = 0;
  static int last_from = -1;
  static int last_to = -1;

  double progress = 0.0;
  int logical_direction = (index > current_index) ? 1 : -1;

  if (_current_animation_group)
  {
    // Try to get progress of the running animation
    QPropertyAnimation *anim = qobject_cast<QPropertyAnimation *>(_current_animation_group->animationAt(0));
    if (anim)
    {
      int elapsed = anim->currentTime();
      progress = qBound(0.0, double(elapsed) / double(_switch_duration_ms), 1.0);
      last_progress = progress;
      last_direction = logical_direction;
      last_from = current_index;
      last_to = index;
    }
    _current_animation_group->stop();
    _current_animation_group->deleteLater();
    _current_animation_group = nullptr;

    // HACK: Instantly jump to the same progress between new pages
    int new_from = current_index;
    int new_to = index;
    int new_direction = (new_to > new_from) ? 1 : -1;
    int from_x = int(-progress * width * last_direction);
    int to_x = from_x + new_direction * width;
    QWidget *from_widget = _ui->stackedWidget->widget(new_from);
    QWidget *to_widget = _ui->stackedWidget->widget(new_to);
    from_widget->setGeometry(from_x, 0, width, area.height());
    to_widget->setGeometry(to_x, 0, width, area.height());
    from_widget->show();
    to_widget->show();
    current_widget = from_widget;
    current_geom = from_widget->geometry();
    current_index = new_from;
    logical_direction = new_direction;
  }

  // Prepare the next widget
  QWidget *next_widget = _ui->stackedWidget->widget(index);
  QRect next_geom = next_widget->geometry();

  // Place next widget at the correct offset from current
  int next_x = current_geom.x() + logical_direction * width;
  next_widget->setGeometry(next_x, 0, width, area.height());
  next_widget->show();

  // Animate both widgets
  auto animate_current = new QPropertyAnimation(current_widget, "geometry");
  animate_current->setDuration(_switch_duration_ms);
  animate_current->setEasingCurve(QEasingCurve::InOutQuad);
  animate_current->setStartValue(current_geom);
  animate_current->setEndValue(QRect(current_geom.x() - logical_direction * width, 0, width, area.height()));

  auto animate_next = new QPropertyAnimation(next_widget, "geometry");
  animate_next->setDuration(_switch_duration_ms);
  animate_next->setEasingCurve(QEasingCurve::InOutQuad);
  animate_next->setStartValue(next_widget->geometry());
  animate_next->setEndValue(QRect(0, 0, width, area.height()));

  _current_animation_group = new QParallelAnimationGroup(this);
  _current_animation_group->addAnimation(animate_current);
  _current_animation_group->addAnimation(animate_next);

  connect(_current_animation_group,
          &QParallelAnimationGroup::finished,
          this,
          [this, index, current_widget, area]()
          {
            _ui->stackedWidget->setCurrentIndex(index);
            current_widget->setGeometry(0, 0, area.width(), area.height());
            _current_animation_group->deleteLater();
            _current_animation_group = nullptr;
            updateWindowTitle();
            _page_queue.removeFirst();
            if (!_page_queue.isEmpty())
              switchPages(_page_queue.first());
          });

  _current_animation_group->start(QAbstractAnimation::DeleteWhenStopped);
  qDebug() << "Page switch animation started";
}

void MainWindow::onButtonReleased(TARGET_T button)
{
  qDebug() << "Button released:" << touch_targets[button].name;

  if (_button_released_promise)
  {
    _button_released_promise->set_value(button);
    _button_released_promise = nullptr;
  }
}

void MainWindow::setButtonReleasedPromise(std::promise<TARGET_T> *promise)
{
  _button_released_promise = promise;
}