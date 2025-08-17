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
#include <algorithm>
#include <future>

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
  if (_is_switching)
  {
    qDebug() << "Already switching pages, ignoring nextPage request.";
    return;
  }

  int count = _ui->stackedWidget->count();
  if (count <= 1)
  {
    qDebug() << "No pages to switch to.";
    return;
  }

  int current_index = _ui->stackedWidget->currentIndex();
  if (current_index >= count - 1)
  {
    qDebug() << "Already at last page, cannot go to next page.";
    return;
  }
  int next_index = current_index + 1;
  switchPages(next_index, 1);
  qDebug() << "Switched to next page:" << next_index;
}

void MainWindow::previousPage()
{
  if (_is_switching)
  {
    qDebug() << "Already switching pages, ignoring previousPage request.";
    return;
  }
  int count = _ui->stackedWidget->count();
  if (count <= 1)
  {
    qDebug() << "No pages to switch to.";
    return;
  }

  int current_index = _ui->stackedWidget->currentIndex();
  if (current_index <= 0)
  {
    qDebug() << "Already at first page, cannot go to previous page.";
    return;
  }
  int previous_index = current_index - 1;
  switchPages(previous_index, -1);
  qDebug() << "Switched to previous page:" << previous_index;
}

void MainWindow::switchPages(int index, int direction)
{
  _is_switching = true;
  qDebug() << "Switching pages to index:" << index << "direction:" << direction;

  int current_index = _ui->stackedWidget->currentIndex();

  QWidget *current_widget = _ui->stackedWidget->widget(current_index);
  QWidget *next_widget = _ui->stackedWidget->widget(index);

  // Get stacked widget area
  QRect area = _ui->stackedWidget->geometry();
  int width = area.width();

  // Move next widget off screen based on direction
  if (direction == 1)
  {
    next_widget->setGeometry(width, 0, width, area.height());
  }
  else
  {
    next_widget->setGeometry(-width, 0, width, area.height());
  }

  // Show next widget
  next_widget->show();

  // Animate the switch
  auto animate_current = new QPropertyAnimation(current_widget, "geometry");
  animate_current->setDuration(_switch_duration_ms);
  animate_current->setEasingCurve(QEasingCurve::InOutQuad);
  if (direction == 1)
  {
    animate_current->setEndValue(QRect(-width, 0, width, area.height()));
  }
  else
  {
    animate_current->setEndValue(QRect(width, 0, width, area.height()));
  }

  auto animate_next = new QPropertyAnimation(next_widget, "geometry");
  animate_next->setDuration(_switch_duration_ms);
  animate_next->setEasingCurve(QEasingCurve::InOutQuad);
  animate_next->setStartValue(next_widget->geometry());
  animate_next->setEndValue(QRect(0, 0, width, area.height()));

  auto animation_group = new QParallelAnimationGroup(this);
  animation_group->addAnimation(animate_current);
  animation_group->addAnimation(animate_next);

  connect(animation_group,
          &QParallelAnimationGroup::finished,
          this,
          [this, index, current_widget, area]()
          {
            // Set index
            _ui->stackedWidget->setCurrentIndex(index);
            // Reset position
            current_widget->setGeometry(0, 0, area.width(), area.height());

            _is_switching = false;

            updateWindowTitle();
            qDebug() << "Page switch animation finished";
          });

  animation_group->start(QAbstractAnimation::DeleteWhenStopped);
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