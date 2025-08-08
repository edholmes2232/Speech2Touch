#pragma once

#include <QMainWindow>
#include <QParallelAnimationGroup>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  private slots:
  void nextPage();
  void previousPage();

  private:
  Ui::MainWindow *ui;

  /**
   * @brief Switch pages with animation.
   * @param index The index of the page to switch to.
   * @param direction The direction to switch (1 for next, -1 for previous).
   */
  void switchPages(int index, int direction);

  bool _is_switching = false;
  const int _switch_duration_ms = 500;

  void setupButtons();
};