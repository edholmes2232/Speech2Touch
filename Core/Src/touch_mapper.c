#include "touch_mapper.h"

#include "log.h"
#include "touch.h"
#include "touch_targets.h"

// Target Screen Dimensions
#define SCREEN_WIDTH_X 800
#define SCREEN_HEIGHT_Y 600

// Target's grid dimensions
#define GRID_COLS 4
#define GRID_ROWS 3
#define GRID_WIDTH 700 // 700 px wide
#define GRID_HEIGHT 470 // 470 px tall

// Macros to map Target Screen Dimensions to output USBD Touch Coordinates
#define TOUCH_TARGET_WIDTH (GRID_WIDTH / GRID_COLS)
#define TOUCH_TARGET_HEIGHT (GRID_HEIGHT / GRID_ROWS)

// Return the X coordinate of the centre of a target in the grid
#define GRID_COL_TO_SCREEN_X(col) (((col) * TOUCH_TARGET_WIDTH) + (TOUCH_TARGET_WIDTH / 2))
// Return the Y coordinate of the centre of a target in the grid
#define GRID_ROW_TO_SCREEN_Y(row) (((row) * TOUCH_TARGET_HEIGHT) + (TOUCH_TARGET_HEIGHT / 2))
// Return the X coordinate of a target in the grid
#define SCREEN_TO_TOUCH_X_COORD(x) (((x) * TOUCH_MAX_X_COORD) / SCREEN_WIDTH_X)
// Return the Y coordinate of a target in the grid
#define SCREEN_TO_TOUCH_Y_COORD(y) (((y) * TOUCH_MAX_Y_COORD) / SCREEN_HEIGHT_Y)

// Targets are arranged in a 4x3 grid of size 700x470
#define COL_TO_SCREEN_X(col) (col * (SCREEN_WIDTH_X / 4))

enum PAGE_DIR
{
  PAGE_DIR_LEFT = -1,
  PAGE_DIR_RIGHT = 1,
};

TOUCH_EVENT_T targetToTouchEvent(TARGET_T target)
{
  if (target == TARGET_NEXT_PAGE)
  {
    static TOUCH_EVENT_T next_page_touch_event = {
        .x = SCREEN_TO_TOUCH_X_COORD(SCREEN_WIDTH_X - 10), // Right edge of the screen
        .y = SCREEN_TO_TOUCH_Y_COORD(SCREEN_HEIGHT_Y / 2), // Middle of the screen
        .delay_ms = 0};
    return next_page_touch_event;
  }

  if (target == TARGET_PREV_PAGE)
  {
    static TOUCH_EVENT_T prev_page_touch_event = {
        .x = SCREEN_TO_TOUCH_X_COORD(10), // Left edge of the screen
        .y = SCREEN_TO_TOUCH_Y_COORD(SCREEN_HEIGHT_Y / 2), // Middle of the screen
        .delay_ms = 0};
    return prev_page_touch_event;
  }

  TOUCH_EVENT_T touch_event = {
      .x = SCREEN_TO_TOUCH_X_COORD(GRID_COL_TO_SCREEN_X(touch_targets[target].col)),
      .y = SCREEN_TO_TOUCH_Y_COORD(GRID_ROW_TO_SCREEN_Y(touch_targets[target].row)),
      .delay_ms = 0,
  };

  log_debug("Mapped col: %d, row: %d to touch coords: (%d, %d)",
            touch_targets[target].col,
            touch_targets[target].row,
            touch_event.x,
            touch_event.y);

  return touch_event;
}

/**
 * @brief Switch pages with animation.
 * @param dir The direction to switch (1 for right, -1 for left).
 * @param num_pages The number of pages to switch.
 */
static void switchPages(enum PAGE_DIR dir, uint8_t num_pages)
{
  TOUCH_EVENT_T touch_event = targetToTouchEvent((dir == PAGE_DIR_LEFT) ? TARGET_PREV_PAGE : TARGET_NEXT_PAGE);

  for (uint8_t i = 0; i < num_pages; i++)
  {

    // Target system can handle multiple page transitions without delay,
    // Only include delay in last page transition
    if (i == (num_pages - 1))
    {
      touch_event.delay_ms = PAGE_TRANSITION_DELAY_MS; // Add a delay for the last page transition
    }
    else
    {
      touch_event.delay_ms = 0; // No delay for intermediate transitions
    }

    TOUCH_QueuePush(&touch_event);
  }
}

void TOUCHMAPPER_ResetState(void)
{
  log_debug("Resetting system state to default");

  // Press "Back" button MAX_PAGE_COUNT times to reset
  switchPages(PAGE_DIR_LEFT, MAX_PAGE_COUNT);
}

void TOUCHMAPPER_HandleTarget(TARGET_T target)
{
  log_debug("Handling target: %s", touch_targets[target].name);

  if (target < 0 || target >= TARGET_COUNT)
  {
    log_error("Invalid target: %d", target);
    return;
  }

  TOUCH_EVENT_T touch_event = targetToTouchEvent(target);
  if ((touch_event.x == 0) && (touch_event.y == 0))
  {
    log_error("No touch event defined for %s", touch_targets[target].name);
    return;
  }

  // Queue any page transition events first
  log_debug("Target is on page %d", touch_targets[target].page);
  switchPages(PAGE_DIR_RIGHT, touch_targets[target].page);

  // Now queue the actual target touch event
  log_debug("Queuing touch event for target: %s", touch_targets[target].name);
  TOUCH_QueuePush(&touch_event);
}
