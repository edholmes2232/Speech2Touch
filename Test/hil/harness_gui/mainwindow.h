#pragma once

#include "touch_targets.h"

#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <future>

// Forward declarations for Qt and Linux-specific types
class QSocketNotifier;
struct input_absinfo;

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

  // For testing: set a promise to be fulfilled on button release
  void setButtonReleasedPromise(std::promise<TARGET_T> *promise);

  private slots:
  void nextPage();
  void previousPage();
  void onButtonReleased(TARGET_T button);
  void updateWindowTitle();
  // Slot to be called by the notifier when the touch device has data
  void readTouchDevice();

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

  std::promise<TARGET_T> *_button_released_promise = nullptr;

  // --- New members for raw touch input handling ---
  void initTouchDevice(); // Initializes the touch device
  void processTouchEvent(); // Helper to process coordinates and post events

  int m_touchFd; // File descriptor for the touch device
  QSocketNotifier *m_notifier; // Notifier for reading data without blocking

  // To store device capabilities and current state
  input_absinfo *m_axisInfoX;
  input_absinfo *m_axisInfoY;
  int m_currentX;
  int m_currentY;
  bool m_isTouched;
  bool m_wasTouched; // To detect press vs. release events
};