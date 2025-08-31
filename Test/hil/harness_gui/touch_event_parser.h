#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <functional>
#include <linux/input.h>

class TouchEventParser : public QObject
{
  Q_OBJECT
  public:
  using TouchCallback = std::function<void(float percent_x, float percent_y, bool is_touched)>;
  TouchEventParser(const char *device_path, TouchCallback callback, QObject *parent = nullptr);
  ~TouchEventParser();

  private slots:
  void readTouchDevice();

  private:
  int _touch_fd;
  QSocketNotifier *_notifier;
  struct input_absinfo _axis_info_x;
  struct input_absinfo _axis_info_y;
  int _current_x;
  int _current_y;
  bool _is_touched;
  bool _was_touched;
  TouchCallback _callback;
};
