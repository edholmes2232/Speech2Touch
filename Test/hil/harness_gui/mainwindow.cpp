#include "mainwindow.h"

#include "touch_targets.h"
#include "ui_Franke-A600.h"

#include <QDebug>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QRect>

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

  // // Sort buttons numerically based on the number in their objectName
  // std::sort(buttons.begin(),
  //           buttons.end(),
  //           [](QPushButton *a, QPushButton *b)
  //           {
  //             // Extracts the number after the '_' (e.g., "12" from "pushButton_12")
  //             int num_a = a->objectName().split('_').last().toInt();
  //             int num_b = b->objectName().split('_').last().toInt();
  //             return num_a < num_b;
  //           });

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
{
  ui->setupUi(this);

  setupButtons();

  // Always start on page 0
  ui->stackedWidget->setCurrentIndex(0);
  updateWindowTitle();

  connect(ui->arrow_right_button, &QPushButton::clicked, this, &MainWindow::nextPage);
  connect(ui->arrow_left_button, &QPushButton::clicked, this, &MainWindow::previousPage);

  qDebug() << "MainWindow initialized";
}

// Helper to update window title
void MainWindow::updateWindowTitle()
{
  int current = ui->stackedWidget->currentIndex() + 1;
  int total = ui->stackedWidget->count();
  setWindowTitle(QString("Franke A600 - Page %1 of %2").arg(current).arg(total));
}

MainWindow::~MainWindow()
{
  delete ui;
  qDebug() << "MainWindow destroyed";
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