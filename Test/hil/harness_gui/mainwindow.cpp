#include "mainwindow.h"

#include "touch_targets.h"
#include "ui_Franke-A600.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QRect>
#include <QSocketNotifier>

// === Required Linux Headers for Raw Input ===
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
// ============================================

// ##########################################################################
// # CRITICAL INSTRUCTIONS FOR TOUCH INPUT                                  #
// ##########################################################################
// # 1. DEVICE PATH: You MUST change "/dev/input/event4" below to the       #
// #    correct event device for your touchscreen. Find it by running       #
// #    `cat /proc/bus/input/devices` in your terminal.                     #
// #                                                                        #
// # 2. PERMISSIONS: To perform an exclusive grab, this application MUST be #
// #    run with root privileges. For testing, use `sudo`:                  #
// #    `sudo ./your_application_name`                                      #
// ##########################################################################

void MainWindow::setupButtons()
{
  for (size_t i = 0; i < TARGET_COUNT; ++i)
  {
    TOUCH_TARGET_T target = touch_targets[i];
    qDebug() << "Setting up button for target:" << target.name;
  }

  // Get all QPushButton children from StackedWidget
  QList<QPushButton *> buttons = ui->stackedWidget->findChildren<QPushButton *>();

  // Sort buttons by name
  std::sort(
      buttons.begin(), buttons.end(), [](QPushButton *a, QPushButton *b) { return a->objectName() < b->objectName(); });

  // Set button names, starting at 0
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_touchFd(-1)
    , m_notifier(nullptr)
    , m_axisInfoX(new input_absinfo)
    , m_axisInfoY(new input_absinfo)
    , m_currentX(0)
    , m_currentY(0)
    , m_isTouched(false)
    , m_wasTouched(false)
{
  ui->setupUi(this);

  setupButtons();

  // Initialize the raw touch device input
  initTouchDevice();

  // Always start on page 0
  ui->stackedWidget->setCurrentIndex(0);
  updateWindowTitle();

  connect(ui->arrow_right_button, &QPushButton::clicked, this, &MainWindow::nextPage);
  connect(ui->arrow_left_button, &QPushButton::clicked, this, &MainWindow::previousPage);

  qDebug() << "MainWindow initialized";
}

MainWindow::~MainWindow()
{
  if (m_touchFd >= 0)
  {
    // IMPORTANT: Release the grab before closing the application
    ioctl(m_touchFd, EVIOCGRAB, 0);
    ::close(m_touchFd);
  }
  delete m_axisInfoX;
  delete m_axisInfoY;
  delete ui;
  qDebug() << "MainWindow destroyed";
}

void MainWindow::initTouchDevice()
{
  // *** IMPORTANT: REPLACE WITH YOUR DEVICE'S PATH ***
  const char *devicePath = "/dev/input/event10";

  m_touchFd = open(devicePath, O_RDONLY | O_NONBLOCK);

  if (m_touchFd < 0)
  {
    qWarning() << "FATAL: Failed to open touch device:" << devicePath << ". Check path and permissions.";
    return;
  }

  // Grab the device for exclusive access to hide it from the OS
  if (ioctl(m_touchFd, EVIOCGRAB, 1) < 0)
  {
    qWarning() << "FATAL: EVIOCGRAB failed. Are you running with sudo?";
    ::close(m_touchFd);
    m_touchFd = -1;
    return;
  }

  // Get the valid range of X and Y absolute coordinates from the driver
  if (ioctl(m_touchFd, EVIOCGABS(ABS_X), m_axisInfoX) < 0 || ioctl(m_touchFd, EVIOCGABS(ABS_Y), m_axisInfoY) < 0)
  {
    qWarning() << "FATAL: Could not get axis info from touch device.";
    ioctl(m_touchFd, EVIOCGRAB, 0); // Release grab
    ::close(m_touchFd);
    m_touchFd = -1;
    return;
  }

  qDebug() << "Successfully grabbed touch device:" << devicePath;
  qDebug() << "Touch X Range:" << m_axisInfoX->minimum << "-" << m_axisInfoX->maximum;
  qDebug() << "Touch Y Range:" << m_axisInfoY->minimum << "-" << m_axisInfoY->maximum;

  // Create a notifier that triggers when there's data to read
  m_notifier = new QSocketNotifier(m_touchFd, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this, &MainWindow::readTouchDevice);
}

void MainWindow::readTouchDevice()
{
  qDebug() << "Reading touch device data...";
  struct input_event event;

  // Read all available event data from the device
  while (read(m_touchFd, &event, sizeof(struct input_event)) > 0)
  {
    qDebug() << "> Event: type=" << event.type << ", code=" << event.code << ", value=" << event.value;
    if (event.type == EV_ABS)
    { // Absolute coordinate change
      if (event.code == ABS_X)
      {
        m_currentX = event.value;
        qDebug() << ">  ABS_X updated:" << m_currentX;
      }
      else if (event.code == ABS_Y)
      {
        m_currentY = event.value;
        qDebug() << ">  ABS_Y updated:" << m_currentY;
      }
    }
    else if (event.type == EV_KEY)
    { // A "key" or "button" press
      if (event.code == BTN_TOUCH)
      {
        m_isTouched = event.value;
        qDebug() << ">  BTN_TOUCH updated:" << m_isTouched;
      }
    }
    else if (event.type == EV_SYN && event.code == SYN_REPORT)
    {
      qDebug() << ">  SYN_REPORT received, processing touch event.";
      // Sync event means a "packet" of data is complete. Process it now.
      processTouchEvent();
    }
  }
}

void MainWindow::processTouchEvent()
{
  // --- 1. Normalize Coordinates to Percentage (0.0 to 1.0) ---
  float percent_x =
      static_cast<float>(m_currentX - m_axisInfoX->minimum) / (m_axisInfoX->maximum - m_axisInfoX->minimum);
  float percent_y =
      static_cast<float>(m_currentY - m_axisInfoY->minimum) / (m_axisInfoY->maximum - m_axisInfoY->minimum);

  qDebug() << "\nNormalized coordinates: (" << percent_x << "," << percent_y << ")";

  // --- 2. Map Percentage to Window Coordinates ---
  QPointF localPos(percent_x * this->width(), percent_y * this->height());
  qDebug() << "Mapped to window coordinates:" << localPos;

  // --- 3. Determine Event Type and Post It ---
  QEvent::Type eventType;
  Qt::MouseButton button = Qt::LeftButton;
  Qt::MouseButtons buttons;

  if (!m_wasTouched && m_isTouched)
  {
    // PRESS: Was not touched, but now is.
    eventType = QEvent::MouseButtonPress;
    buttons = Qt::LeftButton;
    qDebug() << "Touch event: MouseButtonPress at" << localPos;
  }
  else if (m_wasTouched && !m_isTouched)
  {
    // RELEASE: Was touched, but now is not.
    eventType = QEvent::MouseButtonRelease;
    buttons = Qt::NoButton;
    qDebug() << "Touch event: MouseButtonRelease at" << localPos;
  }
  else if (m_isTouched)
  {
    // MOVE: Was touched and still is.
    eventType = QEvent::MouseMove;
    buttons = Qt::LeftButton;
    qDebug() << "Touch event: MouseMove at" << localPos;
  }
  else
  {
    // No event to process if there's no touch
    qDebug() << "No touch event to process.";
    return;
  }

  // Find child widget at position
  QWidget *child_widget = this->childAt(localPos.toPoint());
  if (child_widget)
  {
    qDebug() << "Touch event target widget:" << child_widget->objectName();
  }
  else
  {
    qDebug() << "No widget found at touch position, using MainWindow.";
    child_widget = this; // Use MainWindow as fallback
  }

  // Map local position to child widget's local coordinates
  QPoint widget_local_position = child_widget->mapFrom(this, localPos.toPoint());
  qDebug() << "Local position for event:" << widget_local_position;
  QPoint global_position = child_widget->mapToGlobal(widget_local_position);
  qDebug() << "Global position for event:" << global_position;

  // Post the event to the target widget
  QMouseEvent *mouseEvent =
      new QMouseEvent(eventType, widget_local_position, global_position, button, buttons, Qt::NoModifier);
  QCoreApplication::postEvent(child_widget, mouseEvent);

  // Update state for the next event
  m_wasTouched = m_isTouched;
}

// Helper to update window title
void MainWindow::updateWindowTitle()
{
  int current = ui->stackedWidget->currentIndex() + 1;
  int total = ui->stackedWidget->count();
  setWindowTitle(QString("Franke A600 - Page %1 of %2").arg(current).arg(total));
}

void MainWindow::nextPage()
{
  if (_is_switching)
  {
    qDebug() << "Already switching pages, ignoring nextPage request.";
    return;
  }

  int count = ui->stackedWidget->count();
  if (count <= 1)
  {
    qDebug() << "No pages to switch to.";
    return;
  }

  int currentIndex = ui->stackedWidget->currentIndex();
  if (currentIndex >= count - 1)
  {
    qDebug() << "Already at last page, cannot go to next page.";
    return;
  }
  int nextIndex = currentIndex + 1;
  switchPages(nextIndex, 1);
  qDebug() << "Switched to next page:" << nextIndex;
}

void MainWindow::previousPage()
{
  if (_is_switching)
  {
    qDebug() << "Already switching pages, ignoring previousPage request.";
    return;
  }
  int count = ui->stackedWidget->count();
  if (count <= 1)
  {
    qDebug() << "No pages to switch to.";
    return;
  }

  int currentIndex = ui->stackedWidget->currentIndex();
  if (currentIndex <= 0)
  {
    qDebug() << "Already at first page, cannot go to previous page.";
    return;
  }
  int previousIndex = currentIndex - 1;
  switchPages(previousIndex, -1);
  qDebug() << "Switched to previous page:" << previousIndex;
}

void MainWindow::switchPages(int index, int direction)
{
  _is_switching = true;
  qDebug() << "Switching pages to index:" << index << "direction:" << direction;

  int currentIndex = ui->stackedWidget->currentIndex();

  QWidget *currentWidget = ui->stackedWidget->widget(currentIndex);
  QWidget *nextWidget = ui->stackedWidget->widget(index);

  // Get stacked widget area
  QRect area = ui->stackedWidget->geometry();
  int width = area.width();

  // Move next widget off screen based on direction
  if (direction == 1)
  {
    nextWidget->setGeometry(width, 0, width, area.height());
  }
  else
  {
    nextWidget->setGeometry(-width, 0, width, area.height());
  }

  // Show next widget
  nextWidget->show();

  // Animate the switch
  auto animate_current = new QPropertyAnimation(currentWidget, "geometry");
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

  auto animate_next = new QPropertyAnimation(nextWidget, "geometry");
  animate_next->setDuration(_switch_duration_ms);
  animate_next->setEasingCurve(QEasingCurve::InOutQuad);
  animate_next->setStartValue(nextWidget->geometry());
  animate_next->setEndValue(QRect(0, 0, width, area.height()));

  auto animationGroup = new QParallelAnimationGroup(this);
  animationGroup->addAnimation(animate_current);
  animationGroup->addAnimation(animate_next);

  connect(animationGroup,
          &QParallelAnimationGroup::finished,
          this,
          [this, index, currentWidget, area]()
          {
            // Set index
            ui->stackedWidget->setCurrentIndex(index);
            // Reset position
            currentWidget->setGeometry(0, 0, area.width(), area.height());

            _is_switching = false;

            updateWindowTitle();
            qDebug() << "Page switch animation finished";
          });

  animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
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