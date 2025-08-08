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

  // Set button names, starting at 0
  for (int i = 0; i < buttons.size(); ++i)
  {
    buttons[i]->setObjectName(QString::number(i));
    buttons[i]->setText(QString::number(i));
    // buttons[i]->setText(touch_targets[i].name);
    // qDebug() << "Button" << i << "set to" << touch_targets[i].name;
  }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setupButtons();

  connect(ui->arrow_right_button, &QPushButton::clicked, this, &MainWindow::nextPage);
  connect(ui->arrow_left_button, &QPushButton::clicked, this, &MainWindow::previousPage);

  qDebug() << "MainWindow initialized";
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
  int nextIndex = (currentIndex + 1) % count;
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
  int previousIndex = (currentIndex - 1 + count) % count;
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

            qDebug() << "Page switch animation finished";
          });

  animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
  qDebug() << "Page switch animation started";
}
