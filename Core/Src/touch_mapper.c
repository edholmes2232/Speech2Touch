#include "touch_mapper.h"

#include "log.h"
#include "touch.h"
#include "touch_targets.h"

#define SCREEN_WIDTH_X 800
#define SCREEN_HEIGHT_Y 600

#define SCREEN_TO_TOUCH_X_COORD(x) ((x * TOUCH_MAX_X_COORD) / SCREEN_WIDTH_X)

#define SCREEN_TO_TOUCH_Y_COORD(y) ((y * TOUCH_MAX_Y_COORD) / SCREEN_HEIGHT_Y)

TOUCH_EVENT_T targetToTouchEvent(TARGET_T target)
{
  if (target == TARGET_NEXT_PAGE)
  {
    static TOUCH_EVENT_T next_page_touch_event = {
        .x = SCREEN_WIDTH_X - 10, // Right edge of the screen
        .y = SCREEN_TO_TOUCH_Y_COORD(SCREEN_HEIGHT_Y / 2), // Middle of the screen
        .delay_ms = PAGE_TRANSITION_DELAY_MS};
    return next_page_touch_event;
  }

  if (target == TARGET_PREV_PAGE)
  {
    static TOUCH_EVENT_T prev_page_touch_event = {
        .x = 10, // Left edge of the screen
        .y = SCREEN_TO_TOUCH_Y_COORD(SCREEN_HEIGHT_Y / 2), // Middle of the screen
        .delay_ms = PAGE_TRANSITION_DELAY_MS};
    return prev_page_touch_event;
  }

  TOUCH_EVENT_T touch_event = {0};

  return touch_event;
}

void TOUCHMAPPER_ResetState(void)
{
  log_debug("Resetting system state to default");

  // Press "Back" button MAX_PAGE_COUNT times to reset
  for (int i = 0; i < MAX_PAGE_COUNT; i++)
  {
    TOUCH_EVENT_T back_touch_event = targetToTouchEvent(TARGET_PREV_PAGE);
    TOUCH_QueuePush(&back_touch_event);
  }
}
